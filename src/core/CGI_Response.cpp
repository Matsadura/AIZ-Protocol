#include "CGI_Response.hpp"

CgiOutputParser::CgiOutputParser() : m_state(STATE_HEADERS), m_max_buffer_size(64 * 1024), m_eof_signaled(false)
{
}

CgiOutputParser::~CgiOutputParser()
{
}

void CgiOutputParser::append(const char *data, std::size_t size)
{
    if (size > 0 && data != NULL)
    {
        m_raw_buffer.insert(m_raw_buffer.end(), data, data + size);
        process();
    }
}

void CgiOutputParser::setEof()
{
    m_eof_signaled = true;
    process();
}

bool CgiOutputParser::isBufferFull() const
{
    return m_body_buffer.size() >= m_max_buffer_size || (m_eof_signaled && !isComplete());
}

bool CgiOutputParser::isComplete() const
{
    return m_state == STATE_FINISHED;
}

void CgiOutputParser::reset()
{
    m_state        = STATE_HEADERS;
    m_eof_signaled = false;
    m_raw_buffer.clear();
    m_body_buffer.clear();
    m_headers.clear();
}

bool CgiOutputParser::areHeadersParsed() const
{
    return m_state >= STATE_BODY;
}

const std::string &CgiOutputParser::getHeaders() const
{
    return m_headers;
}

void CgiOutputParser::getBodyData(const char *&out_data, std::size_t &out_size) const
{
    if (m_body_buffer.empty())
    {
        out_data = NULL;
        out_size = 0;
    }
    else
    {
        out_data = &m_body_buffer[0];
        out_size = m_body_buffer.size();
    }
}

void CgiOutputParser::consume(std::size_t written)
{
    if (written == 0)
    {
        return;
    }

    if (written > m_body_buffer.size())
    {
        written = m_body_buffer.size();
    }

    // Erase the consumed bytes from the front.
    // std::vector::erase is highly optimized for contiguous blocks.
    m_body_buffer.erase(m_body_buffer.begin(), m_body_buffer.begin() + static_cast<int>(written));

    // Attempt to parse more data from the raw buffer now that space is available
    process();
}

void CgiOutputParser::process()
{
    if (m_state == STATE_HEADERS)
    {
        const char crlfcrlf[] = "\r\n\r\n";
        const char lflf[]     = "\n\n";

        std::vector<char>::iterator crlf_it =
            std::search(m_raw_buffer.begin(), m_raw_buffer.end(), crlfcrlf, crlfcrlf + 4);
        std::vector<char>::iterator lf_it = std::search(m_raw_buffer.begin(), m_raw_buffer.end(), lflf, lflf + 2);

        std::vector<char>::iterator header_end       = m_raw_buffer.end();
        std::size_t                 delimiter_length = 0;

        if (crlf_it != m_raw_buffer.end() && lf_it != m_raw_buffer.end())
        {
            if (crlf_it < lf_it)
            {
                header_end       = crlf_it;
                delimiter_length = 4;
            }
            else
            {
                header_end       = lf_it;
                delimiter_length = 2;
            }
        }
        else if (crlf_it != m_raw_buffer.end())
        {
            header_end       = crlf_it;
            delimiter_length = 4;
        }
        else if (lf_it != m_raw_buffer.end())
        {
            header_end       = lf_it;
            delimiter_length = 2;
        }

        if (header_end != m_raw_buffer.end())
        {
            // Extract headers and transition state
            m_headers.assign(m_raw_buffer.begin(), header_end);
            m_raw_buffer.erase(m_raw_buffer.begin(), header_end + delimiter_length);
            m_state = STATE_BODY;
        }
    }

    if (m_state == STATE_BODY)
    {
        std::size_t current_body_size = m_body_buffer.size();

        // Move data from raw_buffer_ to body_buffer_ until the limit is reached.
        // It's allowed to temporarily sit at exactly max_buffer_size_.
        if (current_body_size < m_max_buffer_size && !m_raw_buffer.empty())
        {
            std::size_t capacity      = m_max_buffer_size - current_body_size;
            std::size_t bytes_to_move = std::min(capacity, m_raw_buffer.size());

            m_body_buffer.insert(m_body_buffer.end(), m_raw_buffer.begin(), m_raw_buffer.begin() + bytes_to_move);
            m_raw_buffer.erase(m_raw_buffer.begin(), m_raw_buffer.begin() + bytes_to_move);
        }
    }

    // Check for final completion
    if (m_eof_signaled && m_raw_buffer.empty() && m_body_buffer.empty())
    {
        m_state = STATE_FINISHED;
    }
}
