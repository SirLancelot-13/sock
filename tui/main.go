package main

import (
	"fmt"
	"os"
	"regexp"
	"strings"

	tea "github.com/charmbracelet/bubbletea"
)

type Model struct {
	cursor      int
	msgList     map[string]string
	msgListKeys []string //As shitty as this is, this is temporary, ideally I'd refactor the schema to include a timestamp.
	message     string
}

func (m Model) Init() tea.Cmd {
	return nil
}

func loadInitialMessages() Model {
	return Model{
		cursor: 0,
		msgList: map[string]string{
			"1": "Welcome to Socket Chat TUI!", //TODO: Add cgo-based message fetching endpoint from client.c
		},
		msgListKeys: []string{"1"},
		message:     "",
	}
}

func (m Model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	switch msg := msg.(type) {
	case tea.KeyMsg:
		switch msg.String() {
		case "ctrl+c", "esc":
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
				key := fmt.Sprintf("%d", len(m.msgList)+1)
				m.msgList[key] = m.message
				m.msgListKeys = append(m.msgListKeys, key)
				m.message = ""
				m.cursor = 0
			}
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
	s.WriteString("           Random ahh chat thingy          \n")
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
	p := tea.NewProgram(loadInitialMessages())
	if _, err := p.Run(); err != nil {
		fmt.Printf("Error running program: %v\n", err)
		os.Exit(1)
	}
}
