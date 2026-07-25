package main

import (
	"fmt"
	"os"
	"regexp"
	"strings"

	tea "github.com/charmbracelet/bubbletea"
)

// ServerMsg carries incoming WebSocket message payloads received from Cgo
type ServerMsg string

type errMsg struct {
	err error
}

type tickMsg struct{}

type Model struct {
	cursor      int
	msgList     map[string]string
	msgListKeys []string
	message     string
	bridge      *ClientBridge
	status      string
}

// waitForIncomingMessage polls Cgo bridge asynchronously via tea.Cmd
func waitForIncomingMessage(bridge *ClientBridge) tea.Cmd {
	return func() tea.Msg {
		if bridge == nil || !bridge.IsConnected() {
			return nil
		}
		msg, err := bridge.Recv(50) // Poll with 50ms timeout
		if err != nil {
			return errMsg{err: err}
		}
		if msg == "" {
			return tickMsg{}
		}
		return ServerMsg(msg)
	}
}

func loadInitialMessages(bridge *ClientBridge, connErr error) Model {
	status := "Connected"
	initMsg := "Connected to Socket Chat TUI via Cgo!"
	if connErr != nil {
		status = fmt.Sprintf("Disconnected (%v)", connErr)
		initMsg = fmt.Sprintf("Connection status: %v", connErr)
	}

	return Model{
		cursor: 0,
		msgList: map[string]string{
			"1": initMsg,
		},
		msgListKeys: []string{"1"},
		message:     "",
		bridge:      bridge,
		status:      status,
	}
}

func (m Model) Init() tea.Cmd {
	if m.bridge != nil && m.bridge.IsConnected() {
		return waitForIncomingMessage(m.bridge)
	}
	return nil
}

func (m Model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	if m.msgList == nil {
		m.msgList = make(map[string]string)
	}

	switch msg := msg.(type) {

	case ServerMsg:
		key := fmt.Sprintf("%d", len(m.msgList)+1)
		m.msgList[key] = string(msg)
		m.msgListKeys = append(m.msgListKeys, key)
		return m, waitForIncomingMessage(m.bridge)

	case tickMsg:
		return m, waitForIncomingMessage(m.bridge)

	case errMsg:
		m.status = fmt.Sprintf("Disconnected: %v", msg.err)
		return m, nil

	case tea.KeyMsg:
		switch msg.String() {
		case "ctrl+c", "esc":
			if m.bridge != nil {
				m.bridge.Close()
			}
			return m, tea.Quit
		case "backspace":
			if len(m.message) > 0 {
				m.message = m.message[:len(m.message)-1]
				if m.cursor > 0 {
					m.cursor--
				}
			}
		case "left":
			if m.cursor > 0 {
				m.cursor--
			}
		case "right":
			if m.cursor < len(m.message) {
				m.cursor++
			}
		case "enter":
			if m.message != "" {
				if m.bridge != nil && m.bridge.IsConnected() {
					err := m.bridge.Send(m.message)
					if err != nil {
						m.status = fmt.Sprintf("Send error: %v", err)
						return m, nil
					}
				} else {
					m.status = "Error: Not connected to server"
					return m, nil
				}
				key := fmt.Sprintf("%d", len(m.msgList)+1)
				m.msgList[key] = "Me: " + m.message
				m.msgListKeys = append(m.msgListKeys, key)
				m.message = ""
				m.cursor = 0
			}
			return m, waitForIncomingMessage(m.bridge)
		default:
			if match, _ := regexp.MatchString(`^[a-zA-Z0-9[:punct:]\s]$`, msg.String()); match {
				m.message += msg.String()
				m.cursor++
			}
		}
	}
	return m, nil
}

func (m Model) View() string {
	var s strings.Builder

	s.WriteString("===========================================\n")
	s.WriteString("           Socket Chat TUI                 \n")
	s.WriteString(fmt.Sprintf(" Status: %s\n", m.status))
	s.WriteString("===========================================\n\n")

	s.WriteString("Messages:\n")
	if len(m.msgList) == 0 {
		s.WriteString("  (No messages yet)\n")
	} else {
		for _, k := range m.msgListKeys {
			if v := m.msgList[k]; v != "" {
				s.WriteString(fmt.Sprintf("  [%s] %s\n", k, v))
			}
		}
	}

	s.WriteString("\n-------------------------------------------\n")
	s.WriteString(fmt.Sprintf("Input: %s█\n", m.message))
	s.WriteString("-------------------------------------------\n")
	s.WriteString("[Enter] Send message  |  [Ctrl+C / Esc] Quit\n")

	return s.String()
}

func main() {
	bridge, err := NewClientBridge("127.0.0.1", 8080)
	var connErr error
	if err != nil {
		connErr = err
	} else {
		connErr = bridge.Connect("tui_user")
		defer bridge.Close()
	}

	p := tea.NewProgram(loadInitialMessages(bridge, connErr))
	if _, err := p.Run(); err != nil {
		fmt.Printf("Error running program: %v\n", err)
		os.Exit(1)
	}
}
