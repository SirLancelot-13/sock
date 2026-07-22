package main

import (
	tea "github.com/charmbracelet/bubbletea"
)

type Model struct {
	cursor  int
	msgList map[string]string
	message string
}

func (m Model) Init() tea.Cmd {
	return nil
}

func loadInitialMessages() Model {
	return Model{
		cursor: 0,
		msgList: map[string]string{
			"1": "", //TODO: Add cgo-based message fetching endpoint from client.c
		},
		message: "",
	}
}

func (m Model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	switch msg := msg.(type) {
	case tea.KeyMsg:
		switch msg.String() {
		case "left":
			m.cursor--
		case "right":
			m.cursor++
		case "enter":
			//TODO: Send message and shi
			m.cursor = 0
			m.msgList["self"] = m.message //Figure out a better way to handle this
			m.message = ""
		}
		return m, nil
	}
	return m, nil
}

func (m Model) View() string {
	return "yeow"
}

func main() {
	p := tea.NewProgram(Model{})
	_, _ = p.Run()
}
