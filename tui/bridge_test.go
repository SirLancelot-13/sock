package main

import (
	"testing"
	"time"
)

func TestClientBridgeConnectAndSend(t *testing.T) {
	bridge, err := NewClientBridge("127.0.0.1", 8080)
	if err != nil {
		t.Fatalf("Failed to create bridge: %v", err)
	}
	defer bridge.Close()

	err = bridge.Connect("tester")
	if err != nil {
		t.Fatalf("Failed to connect to server: %v", err)
	}

	if !bridge.IsConnected() {
		t.Fatalf("Expected bridge to be connected")
	}

	testMsg := "Hello from automated test!"
	err = bridge.Send(testMsg)
	if err != nil {
		t.Fatalf("Failed to send message: %v", err)
	}

	// Read message broadcast
	recvMsg, err := bridge.Recv(500)
	if err != nil {
		t.Fatalf("Error receiving message: %v", err)
	}

	if recvMsg == "" {
		// Wait slightly and try one more recv
		time.Sleep(50 * time.Millisecond)
		recvMsg, err = bridge.Recv(500)
	}

	t.Logf("Received message from server: %q", recvMsg)
}
