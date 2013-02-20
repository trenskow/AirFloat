//
//  WebConnection.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "Log.h"
#include "WebServer.h"
#include "WebHeaders.h"
#include "WebRequest.h"
#include "WebConnection.h"

WebConnection::WebConnection(Socket* socket, WebServer* server) {
    
    _socket = socket;
    _server = server;
    
    _isConnected = false;
    
    _shouldSelfDestroy = false;
    
    _processRequestCallback = NULL;
    _connectionClosedCallback = NULL;
    _callbackCtx = NULL;
    
}

WebConnection::~WebConnection() {
    
    delete _socket;
    
}

SocketEndPoint* WebConnection::getLocalEndPoint() {
    
    return _socket->GetLocalEndPoint();
    
}

SocketEndPoint* WebConnection::getRemoteEndPoint() {
    
    return _socket->GetRemoteEndPoint();
    
}

bool WebConnection::isConnected() {
    
    _mutex.lock();
    bool ret = _isConnected;
    _mutex.unlock();
    
    return ret;
    
}

void WebConnection::closeConnection() {
    
    if (isConnected())
        _socket->Close();
    
}

void WebConnection::waitConnection() {
    
    if (isConnected())
        pthread_join(_connectionLoopThread, NULL);
    
}

void WebConnection::setProcessRequestCallback(processRequestCallback callback) {
    
    _processRequestCallback = callback;
    
}

void WebConnection::setConnectionClosed(connectionClosedCallback callback) {
    
    _connectionClosedCallback = callback;
    
}

void WebConnection::setCallbackCtx(void* ctx) {
    
    _callbackCtx = ctx;
    
}

void* WebConnection::_connectionLoopKickStarter(void* t) {
    
    pthread_setname_np("Client Connection Socket");
    ((WebConnection*)t)->_connectionLoop();    
    pthread_exit(0);
        
}

void WebConnection::_connectionLoop() {
    
    uint32_t buffersize = 0;
    unsigned char* buffer = NULL;
    uint32_t readpos = 0;
    
    for (;;) {
        
        unsigned char* contentStart;
        int32_t read = 0;
        
        do {
            
            if (READ_SIZE + readpos > buffersize) {
                buffersize += READ_SIZE;
                buffer = (unsigned char*)realloc(buffer, buffersize);
            }
            
            read = _socket->Receive(&buffer[readpos], READ_SIZE);
            
            if (read <= 0)
                break;
            else
                readpos += read;
            
        } while ((contentStart = WebTools::_getContentStart(buffer, readpos)) == NULL);
        
        if (read <= 0)
            break;
        
        unsigned char* headerStart = buffer;
        if ((contentStart = WebTools::_getContentStart(buffer, readpos)) != NULL) { // Find request end
            
            uint32_t headerLength = _convertNewLines(buffer, contentStart - headerStart);
            
            if (headerLength == 2)
                break;
            
            log_data(LOG_INFO, (char*)buffer, headerLength - 1);
            
            char* cmd = (char*) headerStart;
            char* path = NULL;
            char* protocol = NULL;
            while (headerStart[0] != '\n') {
                if (headerStart[0] == ' ') {
                    if (path == NULL)
                        path = (char*)&headerStart[1];
                    else if (protocol == NULL)
                        protocol = (char*)&headerStart[1];
                    headerStart[0] = '\0';
                }
                headerStart++;
                headerLength--;
            }
            
            if (!path || !protocol)
                break;
            
            headerStart[0] = '\0';
            
            headerStart++;
            headerLength--;
            
            WebRequest* request = new WebRequest(cmd, path, protocol);
            request->getHeaders()->_parseHeaders((char*)headerStart, contentStart - headerStart);
            
            const char* contentLengthStr;
            unsigned char* content = NULL;
            uint32_t contentLength = 0;
            if ((contentLengthStr = request->getHeaders()->valueForName("Content-Length")) != NULL && (contentLength = atoi(contentLengthStr)) > 0) {
                
                content = (unsigned char*) malloc(contentLength);
                uint32_t contentReadPos = readpos - (contentStart - buffer);
                memcpy(content, contentStart, contentReadPos);
                
                while (contentReadPos < contentLength) {
                    
                    int32_t contentRead = _socket->Receive(&content[contentReadPos], contentLength - contentReadPos);
                    
                    if (contentRead < 0) {
                        log(LOG_ERROR, "Connection read error");
                        break;
                    }
                    
                    contentReadPos += contentRead;
                    
                }
                
                log(LOG_INFO, "(Content of length: %d bytes)\n", contentLength);
                
            }
            
            assert(_processRequestCallback != NULL);
            
            if (contentLength > 0 && content != NULL)
                request->_setContent(content, contentLength);
                
            _processRequestCallback(this, request, _callbackCtx);
            
            assert(request->_response->_statusCode < 1000);
            
            if (request->_response->_statusCode == 404 && request->_response->_headers->_headerCount == 0)
                request->_response->_headers->addValue("Content-Length", "0");
            
            if (request->_response->_contentLength > 0)
                request->_response->_headers->addValue("Content-Length", "%d", request->_response->_contentLength);
            
            uint32_t protocolLen = strlen(request->getProtocol());
            uint32_t statusCodeLen = 3;
            uint32_t statusMessageLen = strlen(request->_response->_statusMessage);
            
            uint32_t statusLineLen = protocolLen + 1 + statusCodeLen + 1 + statusMessageLen + 2;
            
            uint32_t headersLen = request->_response->_headers->getTotalLength();
            
            uint32_t totalLen = statusLineLen + headersLen;
            
            char* res = (char*)malloc(totalLen);
            
            snprintf(res, totalLen, "%s %d %s\r\n", 
                     request->getProtocol(), 
                     request->_response->_statusCode,
                     request->_response->_statusMessage);
            
            request->_response->_headers->getContent(&res[statusLineLen], totalLen - statusLineLen);
            
            log_data(LOG_INFO, res, totalLen);
            
            _socket->Send(res, totalLen);
            
            free(res);
            
            if (request->_response->_contentLength > 0) {
                log_data(LOG_INFO, (char*) request->_response->_content, request->_response->_contentLength);
                _socket->Send((char*)request->_response->_content, request->_response->_contentLength);
            }
            
            if (!request->_response->_keepAlive)
                _socket->Close();
            
            delete request;
            
            if (content != NULL)
                free(content);
            
        }
        
        free(buffer);
        buffer = NULL;
        buffersize = 0;
        readpos = 0;
        
    }
    
    log(LOG_INFO, "Client disconnected");
    
    _socket->Close();
    
    if (buffer != NULL)
        free(buffer);
    
    _mutex.lock();
    _isConnected = false;
    _mutex.unlock();
    
    _server->_connectionClosed(this);
    
    if (_connectionClosedCallback != NULL)
        _connectionClosedCallback(this, _callbackCtx);
    
    if (_shouldSelfDestroy)
        delete this;
    
}

void WebConnection::_takeOff() {
    
    _mutex.lock();
    
    _isConnected = true;    
    pthread_create(&_connectionLoopThread, NULL, _connectionLoopKickStarter, this);
    
    _mutex.unlock();
    
    char ip[50];
    _socket->GetRemoteEndPoint()->getHost(ip, 50);
    
    log(LOG_INFO, "RAOPConnection (%p) took over connection from %s:%d", this, ip, _socket->GetRemoteEndPoint()->getPort());
    
}
