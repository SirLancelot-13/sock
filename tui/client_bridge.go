package main

/*
#cgo CFLAGS: -I../client -I../functions -I../server
#cgo LDFLAGS: -L../client -lsockclient
#include "../client/client.h"
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"unsafe"
)

// ClientBridge provides a safe Go wrapper around the C sock_client API
type ClientBridge struct {
	ctx *C.sock_client_t
}

// NewClientBridge creates a new client session targeting serverIP:port
func NewClientBridge(serverIP string, port int) (*ClientBridge, error) {
	cIP := C.CString(serverIP)
	defer C.free(unsafe.Pointer(cIP))

	ctx := C.sock_client_create(cIP, C.int(port))
	if ctx == nil {
		return nil, errors.New("failed to allocate client context")
	}
	return &ClientBridge{ctx: ctx}, nil
}

// IsConnected returns true if the bridge has an active context and socket connection
func (c *ClientBridge) IsConnected() bool {
	return c != nil && c.ctx != nil && c.ctx.connected != 0
}

// Connect registers (if username provided) and performs WebSocket handshake
func (c *ClientBridge) Connect(username string) error {
	if c == nil || c.ctx == nil {
		return errors.New("client context is nil")
	}
	cUser := C.CString(username)
	defer C.free(unsafe.Pointer(cUser))

	res := C.sock_client_connect(c.ctx, cUser)
	if res != 0 {
		return errors.New("failed to connect to server (check if server is running on target port)")
	}
	return nil
}

// Send encodes and transmits a WebSocket text frame to the server
func (c *ClientBridge) Send(msg string) error {
	if c == nil || c.ctx == nil {
		return errors.New("client context is nil")
	}
	if c.ctx.connected == 0 {
		return errors.New("cannot send message: disconnected from server")
	}
	cMsg := C.CString(msg)
	defer C.free(unsafe.Pointer(cMsg))

	res := C.sock_client_send_message(c.ctx, cMsg)
	if res != 0 {
		return errors.New("failed to transmit message")
	}
	return nil
}

// Recv polls for incoming WebSocket text frames with a timeout in milliseconds
func (c *ClientBridge) Recv(timeoutMs int) (string, error) {
	if c == nil || c.ctx == nil {
		return "", errors.New("client context is nil")
	}
	if c.ctx.connected == 0 {
		return "", errors.New("disconnected from server")
	}

	cStr := C.sock_client_recv_message(c.ctx, C.int(timeoutMs))
	if cStr == nil {
		if c.ctx.connected == 0 {
			return "", errors.New("connection closed by server")
		}
		return "", nil // Timeout with no message
	}
	defer C.sock_client_free_string(cStr)
	return C.GoString(cStr), nil
}

// Close gracefully closes socket connections and frees C memory
func (c *ClientBridge) Close() {
	if c != nil && c.ctx != nil {
		C.sock_client_destroy(c.ctx)
		c.ctx = nil
	}
}
