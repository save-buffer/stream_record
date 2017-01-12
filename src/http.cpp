#pragma once

#define DEFAULT_PORT "80"

typedef std::unordered_map<std::string, std::string> http_header;

struct http_response
{
    uint64 response_length;
    void *response;
};

struct get_request
{
    std::string request;
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

get_request create_get_request(char *url)
{
    get_request result = {};
    std::regex host_regex("((([A-Za-z])*\\.([A-Za-z])*))+");
    std::cmatch match;
    
    std::regex_search(url, match, host_regex);

    result.host = match.str();
    result.path = match.suffix();
    if(result.path == "")
	result.path = "/";
    
    result.request = "GET ";
    result.request += result.path;
    result.request += " HTTP/1.1\r\nHost: ";
    result.request += result.host;
    result.request += "\r\n\r\n";
    std::cout << result.request;
    
    return(result);
}

http_response http_get(char *url)
{
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
    
    r = getaddrinfo(request.host.c_str(), DEFAULT_PORT, &hints, &result);
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
	response.append(buffer, r);;
    } while (r > 0);

    closesocket(sock);

    printf("total bytes received: %d\n", bytes_received);
    res.response_length = bytes_received;
    res.response = malloc(bytes_received);
    memcpy(res.response, response.c_str(), bytes_received + 1);

    parse_header(response);
    return(res);
}

#undef DEFAULT_PORT
