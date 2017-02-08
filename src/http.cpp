#pragma once

typedef std::unordered_map<std::string, std::string> http_header;

struct http_response
{
    http_header header;
    uint64 response_length;
    void *response;
};

struct host_path_bundle
{
    std::string host;
    std::string path;
};

http_header parse_header(std::string response)
{    
    http_header result;

    uint32 end_idx = response.find("\r\n\r\n");
    std::string response_header = response.substr(0, end_idx + 2);
    std::regex header_field(".+: .+\r\n");
    auto fields = std::sregex_iterator(response_header.begin(), response_header.end(), header_field);
    auto end = std::sregex_iterator();
    for(std::sregex_iterator i = fields; i != end; i++)
    {
	std::string match = (*i).str();
	uint32 colon_index = match.find(":");
	std::string field_name = match.substr(0, colon_index); 
	std::string field_value = match.substr(colon_index + 2, match.size() - 3 - colon_index);

	result.emplace(field_name, field_value);	
    }
    return(result);    
}

std::string header_to_string(http_header header)
{
    std::string result("");
    for(auto i : header)
    {
	result += i.first; 
	result += ": ";
	result += i.second;
	result += "\n";
    }
    return(result);

}

host_path_bundle create_http_request(std::string url)
{
    host_path_bundle result = {};
    std::regex host_regex("((([A-Za-z])*\\.([A-Za-z])*))+");
    std::cmatch match;
    
    std::regex_search(url.c_str(), match, host_regex);

    result.host = match.str();
    result.path = match.suffix();
    if(result.path == "")
	result.path = "/";
    
    return(result);
}

http_response http_request(DWORD port, DWORD request_flags, char *verb, http_header header, std::string url, std::string payload)
{

    void *void_payload = (void *)payload.c_str();
    int payload_length = payload.length() + 1;
    if(payload == "")
    {
	void_payload = 0;
	payload_length = 0;
    }
    http_response res;
    
    host_path_bundle my_request = create_http_request(url);
    std::string header_as_string = header_to_string(header);
    HINTERNET open = InternetOpen("streamrecord", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
    HINTERNET connect = InternetConnect(open, my_request.host.c_str(), port, "", "",
					INTERNET_SERVICE_HTTP, 0, 0);
    HINTERNET request = HttpOpenRequest(connect, verb, my_request.path.c_str(), "HTTP/1.1", 0, 0, request_flags, 0);
    HttpSendRequest(request, header_as_string.c_str(), header_as_string.length(), void_payload, payload_length);


    char buffer[256];
    char headers[2048];
    DWORD header_length = 2048;
    HttpQueryInfo(request, HTTP_QUERY_RAW_HEADERS_CRLF, headers, &header_length, 0);
    std::string header_string(headers);
    res.header = parse_header(header_string);
    
    std::string response;
    int32 bytes_received = 0;
    DWORD r = 0;
    BOOL done;
    do
    {
	done = InternetReadFile(request, buffer, 256, &r);
	if(r > 0)
	    printf("received %d bytes\n", r);
	else if(r == 0)
	    printf("connection closed\n");
	else
	    printf("recv failed\n");
	bytes_received += r;
	response.append(buffer, r);
    } while (r != 0);
    res.response_length = bytes_received;
    res.response = malloc(res.response_length);
    memcpy(res.response, response.c_str(), res.response_length);
    return(res);

}

http_response https_get(std::string url, http_header header = http_header(), std::string payload = "")
{
    return(http_request(INTERNET_DEFAULT_HTTPS_PORT, INTERNET_FLAG_SECURE, "GET", header, url, payload));
}

http_response http_get(std::string url, http_header header = http_header(), std::string payload = "")
{
    return(http_request(INTERNET_DEFAULT_HTTP_PORT, 0, "GET", header, url, payload));
    /*
    http_response res = {};
        
    std::string response = "";
    get_request request= create_get_request(url);

    char buffer[256];
    SOCKET sock = INVALID_SOCKET;
    addrinfo *result = NULL, *ptr = NULL, hints;
    int r;

    WSADATA data;
    r = WSAStartup(MAKEWORD(2, 2), &data);
    if(r != 0)
    {
	printf("WSAStartup failed\n");
	return(res);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    r = getaddrinfo(request.host.c_str(), "80", &hints, &result);
    if(r != 0)
    {
	printf("getaddrinfo() failed with error %d\n", r);
	WSACleanup();
	return(res);
    }
    
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
	sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	r = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
	if(r == SOCKET_ERROR)
	{
	    closesocket(sock);
	    sock = INVALID_SOCKET;
	    continue;
	}
	break;
    }

    freeaddrinfo(result);

    if(sock == INVALID_SOCKET)
    {
	printf("invalid socket\n");
	WSACleanup();
	return(res);
    }
    
    r = send(sock, request.request.c_str(), request.request.size() + 1, 0);
    if(r == SOCKET_ERROR)
    {
	printf("send failed\n");
	closesocket(sock);
	WSACleanup();
	return(res);
    }

    r = shutdown(sock, SD_SEND);
    if(r == SOCKET_ERROR)
    {
	printf("shutdown failed\n");
	closesocket(sock);
	WSACleanup();
	return(res);
    }

    int32 bytes_received = 0;
    do
    {
	r = recv(sock, (char *)buffer, 256, 0);
	if(r > 0)
	    printf("received %d bytes\n", r);
	else if(r == 0)
	    printf("connection closed\n");
	else
	    printf("recv failed\n");
	bytes_received += r;
	response.append(buffer, r);
    } while (r > 0);

    closesocket(sock);
 
    printf("total bytes received: %d\n", bytes_received);

    uint32 header_length = response.find("\r\n\r\n") + 4;
    res.header = parse_header(response);
    if(res.header["Content-Length"] == "")
    {
	res.response_length = response.find("\r\n\r\n", header_length) - header_length;
    }
    else
    {
	res.response_length = std::stoi(res.header["Content-Length"]);
    }
    res.response = malloc(res.response_length);
    memcpy(res.response, response.c_str() + header_length, res.response_length);
    return(res);
    */
}
