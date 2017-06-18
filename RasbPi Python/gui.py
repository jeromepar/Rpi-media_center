import urwid
import cmd_manager
import mc_utils
import sys
import time

def question(cmd=''):
    return urwid.Pile([urwid.Edit(('input',u" ? "),cmd)])

def set_last_position(obj):
    obj.focus_position = len(obj.body)-1

def update_hour(mainLoop, En_tete):
    En_tete.set_text(time.strftime('%d/%m/%y %H:%M',time.localtime()))
    mainLoop.set_alarm_in(10, update_hour, En_tete)


class ConversationListBox(urwid.ListBox):
    def __init__(self):
	self.params = mc_utils.read_conf("/home/pi/media_center/mc.conf")
	self.database =  mc_utils.generate_database(self.params)

	self.list_cmd_send=['']
	self.pos_in_history = 0

	body = urwid.SimpleFocusListWalker(
		[mc_utils.widget_text("",'info'),question()])
	super(ConversationListBox, self).__init__(body)

    def keypress(self, size, key):
	if key == 'ctrl d':
	    raise urwid.ExitMainLoop()

	elif key == '*':
	    previous_cmd = self.list_cmd_send[1]
	    mc_utils.print_txt(self, previous_cmd, 'error')
#	    (match, lead, season, episode) = mc_utils.is_pattern_S_E(previous_cmd)
#	    if match:
#		command = lead+"S"+str(season)+"E"+str(episode+1)
	    (match, lead, episode) = mc_utils.is_pattern_E(previous_cmd)
	    (match2, lead2, episode2) = mc_utils.is_pattern_xx(previous_cmd)

	    if match:
		numb=str(episode+1)
		if len(numb)==1:
		    numb="0"+numb
		command = lead+"E"+numb
		mc_utils.print_txt(self, "next cmd : "+command, 'info')
	    elif match2:
		numb=str(episode2+1)
		command = lead2+numb
		mc_utils.print_txt(self, "next cmd : "+command, 'info')
	    else:
		mc_utils.print_txt(self, "pattern not found", 'error')
		if self.mainFrame.body == self :
		    self.body.append(question())
		    set_last_position(self)
		return
	    out = cmd_manager.interpret_cmd(self, self.database, self.params, command)

	    self.list_cmd_send.insert(1, command)
	    if len(self.list_cmd_send)>51:
		self.list_cmd_send.pop()
	    self.pos_in_history=0

	    if self.mainFrame.body == self :
		self.body.append(question())
		set_last_position(self)
	    return

	elif key == 'up':
	    self.pos_in_history = self.pos_in_history+1
	    if self.pos_in_history >= len(self.list_cmd_send):
		self.pos_in_history = len(self.list_cmd_send)-1
	    else:
		self.focus[0].set_edit_text(self.list_cmd_send[self.pos_in_history])
		self.focus[0].edit_pos = len(self.list_cmd_send[self.pos_in_history])
	    return key
	elif key == 'down':
	    self.pos_in_history = self.pos_in_history-1
	    if self.pos_in_history < 0:
		self.pos_in_history = 0
	    else:
		self.focus[0].set_edit_text(self.list_cmd_send[self.pos_in_history])
		self.focus[0].edit_pos = len(self.list_cmd_send[self.pos_in_history])
	    return key
	elif key == 'tab':
	    cmd = self.focus[0].edit_text.strip()
	    word = cmd.split(' ')[-1]
	    result = cmd_manager.interpret_cmd(self, self.database, self.params, "f "+word)

	    self.body.append(question(cmd))
	    set_last_position(self)

	    self.pos_in_history=0
	    self.list_cmd_send[self.pos_in_history]=cmd
	    self.focus[0].edit_pos = len(self.list_cmd_send[self.pos_in_history])

	key = super(ConversationListBox, self).keypress(size, key)
	if key != 'enter':
	    return key
	elif key == 'enter':
	    #Appeller la commande
	    command = self.focus[0].edit_text
	    if command == u'exit':
		raise urwid.ExitMainLoop()

	    out = cmd_manager.interpret_cmd(self, self.database, self.params, command)
	    if type(out)==type(list()):
		for o in out:
		    mc_utils.print_txt(self, o,'multiple_cmd')
		    cmd_manager.interpret_cmd(self, self.database, self.params, o)

	    if command != '':
		self.list_cmd_send.insert(1, command)
		if len(self.list_cmd_send)>51:
		    self.list_cmd_send.pop()
		self.pos_in_history=0

		if self.mainFrame.body == self :
		    self.body.append(question())
		    set_last_position(self)

palette = [('input', 'white,bold', 'black'),
	('head','light gray','black'),
	('error','light red','black'),
	('multiple_cmd','light magenta','black'),
	('choice','light magenta','dark blue'),
	('title_choice','white,bold','dark blue'),
	('background_choice','default','dark blue'),
	('out','light blue','black'),
	('info','dark green','black'),]

en_tete = urwid.Text(time.strftime('%d/%m/%y %H:%M',time.localtime()), align='right')
filler_en_tete = urwid.AttrMap(en_tete, 'head')
pied = urwid.Text("----------------------",align='center')
filler_pied = urwid.AttrMap(pied, 'head')
maConversation = ConversationListBox()

mainFrame = urwid.Frame(maConversation,filler_en_tete)
maConversation.mainFrame=mainFrame
mainLoop = urwid.MainLoop(mainFrame, palette)
maConversation.mainLoop = mainLoop
maConversation.en_tete = en_tete

mainLoop.set_alarm_in(0, update_hour, en_tete)

mainLoop.run()
