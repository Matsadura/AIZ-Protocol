#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

class Request {
    public:

        enum ParserState {
            REQUEST_LINE,
            HEADERS,
            BODY,
            COMPLETE,
            ERROR
        };

        enum ChunkState {
            CHUNK_SIZE,
            CHUNK_DATA,
            CHUNK_TRAILER
        };
        
        Request( void );
        Request(const Request& other);
        Request& operator=(const Request& other);
        ~Request( void );

        void 								appendData(const char* data, size_t length);
        
        ParserState 						getState() const;
        int 								getErrorCode() const;
        bool								isComplete() const;

        std::string 						getMethod() const;
        std::string 						getURI() const;
        std::string 						getVersion() const;
        std::string 						getHeader(const std::string& key) const;
        std::map<std::string, std::string>	getHeaders() const;
        const std::vector<char>& 			getBody() const;

    private:
        ParserState 						m_state;
        std::string 						m_raw_buffer;
        int 								m_error_code;
        size_t 								m_max_body_size;

        std::string							m_method;
        std::string 						m_uri;
        std::string 						m_path;
        std::string 						m_query;
        std::string 						m_version;
        std::map<std::string, std::string>	m_headers;
        std::vector<char>					m_body;

        bool 								m_is_chunked;
        size_t 								m_content_length;

        ChunkState							m_chunk_state;
        size_t 								m_current_chunk_size;
        size_t 								m_chunk_bytes_read;

        void 								parseRequestLine();
        void 								parseHeaders();
        void 								parseBody();
        void 								parseChunkedBody();

        std::string 						decodeURI(const std::string& uri);
        void 								exctractPathAndQuery();
};

#endif /* REQUEST_HPP */