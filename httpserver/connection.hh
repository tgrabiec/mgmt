//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include "request_handler.hh"
#include "request_parser.hh"
#include "request.hh"
#include "reply.hh"

#include <boost/asio.hpp>
#include <array>
#include <memory>
#include <iostream>
#include <fstream>

namespace http {

namespace server {

typedef std::array<char, 8192> buffer_type;

class connection_manager;

class multipart_parser {
public:
    multipart_parser();
    void set_boundary(const std::string& _boundary);

    request_parser::result_type parse(request& req, buffer_type::pointer & bg,
                                      const buffer_type::pointer & end);
    void close()
    {
        upload_file.close();
    }

    bool is_done() {
        return mode == DONE;
    }
private:
    enum reading_mode {
        WAIT_BOUNDARY,
        WAIT_CONTENT_DISPOSITION,
        WAIT_EMPTY,
        WRITE_TO_FILE,
        DONE
    };

    void set_original_file(request& req, const std::string val);

    void open_tmp_file(request& req);

    reading_mode mode;

    size_t empty_lines;

    size_t pos_in_file;

    std::string boundary;

    std::ofstream upload_file;

    size_t header_length;
};

/**
 * Represents a single connection from a client.
 *
 */
class connection : public std::enable_shared_from_this<connection> {
public:
    /**
     * Construct a connection with the given socket.
     *
     * @param socket the new socket
     * @param manager the connection manager
     * @param handler the request handler
     */
    explicit connection(boost::asio::ip::tcp::socket socket,
                        connection_manager& manager, request_handler& handler);

    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    /**
     * Start the first asynchronous operation for the connection.
     *
     */
    void start();

    /**
     * Stop all asynchronous operations associated with the connection.
     *
     */
    void stop();

private:
    /**
     * Perform an asynchronous read operation.
     *
     */
    void do_read();

    /**
     * Perform an asynchronous write operation.
     *
     */
    void do_write();

    /**
     * check if the request has content
     * and update it accordingly.
     * @return false on error
     */
    bool set_content_type();

    /**
     * Socket for the connection.
     *
     */
    boost::asio::ip::tcp::socket socket_;

    /**
     * The manager for this connection.
     *
     */
    connection_manager& connection_manager_;

    /**
     * The handler used to process the incoming request.
     *
     */
    request_handler& request_handler_;

    /**
     * Buffer for incoming data.
     *
     */
    buffer_type buffer_;

    /**
     * The incoming request.
     *
     */
    request request_;

    /**
     * The parser for the incoming request.
     *
     */
    request_parser request_parser_;

    /**
     * The reply to be sent back to the client.
     *
     */
    reply reply_;
};

typedef std::shared_ptr<connection> connection_ptr;

} // namespace server

} // namespace http

#endif // HTTP_CONNECTION_HPP
