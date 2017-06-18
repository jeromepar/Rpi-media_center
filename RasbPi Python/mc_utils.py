# -*- coding: utf-8 -*-
import os
import commands
import urwid
import cmd_manager
import re

def read_conf(path_conf):
    path_conf = adjust_path(path_conf)
    file = open(path_conf, 'r')
    etat = ''
    options=''
    dico_path = dict()
    paths = list()
    filetype = list()
    streams = dict()
    aliases=dict()
    out = dict()
    custom_cmds = dict()
    for line in file:
        line = line.strip()
        if line[0] != '#':
            if line[0] == '[':
                etat = line
            elif 'PATH' in etat:
                if '=' in line:
                    element = line.split('=')[0]
                    paths = list()
                    paths.append(line.split('=')[1].strip('\t').strip())
                elif 'end_' + element in line:
		    paths=sorted(paths, key=str.lower)
		    paths=remove_double(paths)
                    dico_path[element] = paths
                else:
                    paths.append(line.strip('\t').strip())
            elif 'FILETYPE' in etat:
                filetype.append(line.strip('\t').strip())
            elif 'ALIAS' in etat:
		alias=line.split('=')[0].strip('\t').strip()
		value=line.split('=')[1].strip('\t').strip()
                aliases[alias]=value
            elif 'OPTIONS' in etat:
		options=line.strip('\t').strip()
            elif 'STREAM' in etat:
		stream=line.split('=')[0].strip('\t').strip()
		value=line.split('=')[1].strip('\t').strip()
                streams[stream]=value
            elif 'EXTERN_COMMANDS' in etat:
		shortcut=line.split('=')[0].strip('\t').strip()
		cmd=line.split('=')[1].strip('\t').strip()
                custom_cmds[shortcut]=cmd

    file.close()
    out["dico_path"]=dico_path
    out["filetype"]=filetype
    out["aliases"]=aliases
    out["play_options"]=options
    out["streams"]=streams
    out["custom_cmds"]=custom_cmds

    return out



def generate_database(params):
    dico_path = adjust_path(params["dico_path"])
    filetype = params["filetype"]

    database = dict()
    for element in dico_path:
        list_file = list()
        for path in dico_path[element]:
            for (dirpath, subdirs, files,) in os.walk(path):
                for x in files:
                    if x.split('.')[-1] in filetype:
                        list_file.append(os.path.join(dirpath, x))
        list_file=sorted(list_file, key=str.lower)
        list_file = remove_double(list_file)
        database[element] = list_file
    return database

def get_name(path):
	name=path.split('/')[-1]
	name=name.split('.')[0:-1]
	name2=''
	for e in name:
		name2=name2+'.'+e
	return name2[1::]

def remove_double(path):
	previous=''
	for p in path:
		if p==previous:
			path.remove(p)
		else:
			previous=p
	return path

def list_files(gui,args, database):
	paths=list_paths(gui,args, database)
	out=list()
	for e in paths:
		out.append(get_name(e))
        out=sorted(out, key=str.lower)
        out=remove_double(out)
	return out

def list_paths(gui,args,database):
	out=list()
	l_arg=len(args)
	if l_arg > 1:
		if args[1] in database:
			print_txt(gui, "Liste de "+args[1]+':','info')
			for e in database[args[1]]:
				out.append(e)
		else:
			string=''
			for e in database:
				string=string+', '+e
			print_txt(gui, "Argument inconnu, doit appartenir à ["+string[2::]+"]",'error')
	else:
		print_txt(gui, "Liste tout par catégorie",'info')
		for type in database:
			for e in database[type]:
				out.append(e)
	
        out=sorted(out, key=str.lower)
        out=remove_double(out)
	return out

def find_name(gui,args, database):
	paths=find_paths(gui,args, database)
	out=list()
	for p in paths:
		out.append(get_name(p))
        out=sorted(out, key=str.lower)
        out=remove_double(out)
	return out

def find_paths(gui, args,database):
	l_arg=len(args)
	out=list()
        if l_arg == 1:
		print_txt(gui, "Erreur: chaine de recherche vide",'error')
	elif l_arg == 2:        
	        for type in database:   
			for e in database[type]:
				name=get_name(e).decode(encoding='UTF8')
				if '&' in args[1]:
					ok=1
					for i in args[1].split('&'):
						if not i.lower() in name.lower():
							ok=0
					if ok:
						out.append(e)
				elif args[1].lower() in name.lower():
					out.append(e)
	elif l_arg == 3:        
		if args[1] in database:
			print_txt(gui, "Recherche dans "+args[1]+':','info')
			for e in database[args[1]]:
				name=get_name(e).decode(encoding='UTF8')
				if '&' in args[2]:
					ok=1
					for i in args[2].split('&'):
						if not i.lower() in name.lower():
							ok=0
					if ok:
						out.append(e)
				elif args[2].lower() in name.lower():
					out.append(e)
		else:
			string=''
			for e in database:
				string=string+', '+e
			print_txt(gui, "Argument inconnu, doit appartenir à ["+string[2::]+"]",'error')
        out=sorted(out, key=str.lower)
        out=remove_double(out)
	for o in out:
	    o = adjust_path(o)
	return out

def find_stream(gui, args, streams):
    for s in streams:
	if args[1] == s:
	    return streams[s]
    print_txt(gui, "stream non trouvé",'error')
    return ''

def interpret_alias(args, aliases):
	out = ''
	for a in args.split(' '):
		if a in aliases:
			out = out + aliases[a]+' '
		else:
			out = out + a +' '
	return out

def play(path, options, fifo_on):
	os.system("echo "+get_name(path)+" >/home/pi/media_center/movie_playing");
	os.system("setterm -cursor off")
	if fifo_on == 1:
	    os.system('omxplayer '+options+' '+adjust_path(path)+
	    	" < /home/pi/media_center/omx_fifo | echo . > /home/pi/media_center/omx_fifo | clear")
	else:
	    os.system('omxplayer '+options+' '+adjust_path(path)+" | clear")
	os.system("rm /home/pi/media_center/movie_playing");
	os.system("setterm -cursor on")

def play_stream(adr, options):
	os.system("setterm -cursor off")
	out=commands.getoutput("livestreamer "+adr+' best -np "omxplayer '+options+'"')
	os.system("setterm -cursor on")
	return out

def choice(gui, str_cmd, choices):
    body = [urwid.Divider(),urwid.Text(('title_choice',"Veuillez choisir parmis les choix suivants:")), urwid.Divider()]
    button = urwid.Button("Cancel")
    urwid.connect_signal(button, 'click', item_chosen, (gui, '','') )
    body.append(urwid.AttrMap(button, 'choice', focus_map='reversed'))

    max_len=39
    for c in choices:
	button = urwid.Button(c)
	urwid.connect_signal(button, 'click', item_chosen, (gui, str_cmd, c) )
	body.append(urwid.AttrMap(button, 'choice', focus_map='reversed'))
	if len(c)>max_len:
	    max_len=len(c)
    content1 = urwid.Padding(urwid.ListBox(urwid.SimpleFocusListWalker(body)), left=2, right=2)
    content2 =  urwid.AttrMap(content1,'background_choice')
    top = urwid.Overlay(content2, gui.mainFrame.body,
	    align='center', width=max_len+4+4,
	    valign='middle', height=(4+len(choices)+1))
    return top

def item_chosen(button, arg):
    gui = arg[0]
    gui.mainFrame.body = gui
    str_cmd = arg[1]
    choice = arg[2]

    if choice != '':
	cmd_manager.interpret_cmd(gui, gui.database, gui.params, str_cmd+' "'+choice+'" -exact')
	gui.pos_in_history=0
	gui.body.append(urwid.Pile([urwid.Edit(('input',u" ? "),'')]))
	gui.focus_position = len(gui.body)-1

def get_subtitle_path(path):
	point_position=path.rfind('.')
	sub_path=path[0:point_position]+".srt"
	return sub_path

def get_path(gui, name, database):
	path = find_paths(gui, ['',name],database)
	path=path.pop()
	return path

def adjust_path(path):
	if ' ' in path:
		path=path.replace(' ','\\ ')
	if "'" in path:
		path=path.replace("'","\\'")
	if "[" in path:
		path=path.replace("[","\\[")
	if "(" in path:
		path=path.replace("(","\\(")
	if ")" in path:
		path=path.replace(")","\\)")
	if "]" in path:
		path=path.replace("]","\\]")
	if "&" in path:
		path=path.replace("&","\\&")
	if "$" in path:
		path=path.replace("$","\\$")
	return path

def widget_text(text,style='info'):
    return urwid.Text((style, text))

def inc_position(obj):
    pos= obj.focus_position
    obj.focus_position = pos + 1

def print_txt(obj, text,style):
    obj.body.append(widget_text(text,style))
    inc_position(obj)

def match(obj, s):
    if type(obj) != type(list()):
	elem = obj
	obj=list()
	obj.append(elem)
    for e in obj:
	s1=s.split('*')[0]
	s2=s.split('*')[1]
	if s1 in e:
	    pos1 = e.find(s1)+len(s1)
	    if s2 in e[pos1+1: len(e)]:
		pos2=e.rfind(s2)
		return e[pos1:pos2]
    return ''

def is_int(s):
    try:
    	int(s)
	return True
    except:
    	return False

def get_video_duration(path):
	path = adjust_path(path)
	out=commands.getoutput("omxplayer -i "+path)
	string=out[out.find("Duration: "):out.find(", start: ")]
	elements = string.split(':')
	time_in_sec=3600*int(elements[1])+60*int(elements[2])+int(float(elements[3]))
	return time_in_sec

def get_video_info(path):
	path = adjust_path(path)
	out=commands.getoutput("omxplayer -i "+path)
	return out

def get_subtitle_codec(path):
    path = adjust_path(path)
    return commands.getoutput("uchardet "+path)

def is_pattern_S_E(s):
    if re.match(".*[sS]\d*[eE]\d*$",s):
	lead = re.split("[sS]\d*[eE]\d*$",s)[0]
	trail = s[len(lead):]
	trail_S = trail.split("E")[0][1:]
	trail_E = trail.split("E")[1]
	return (1, lead,int(trail_S),int(trail_E))
    else:
	return (0,"",0,0)

def is_pattern_E(s):
    if re.match(".*[eE]\d*$",s):
	lead = re.split("[eE]\d*$",s)[0]
	trail = s[len(lead):][1:]
	return (1, lead,int(trail))
    else:
	return (0,"",0)

def is_pattern_xx(s):
    if re.match(".*\d*$",s):
	lead = re.split("\d*$",s)[0]
	trail = s[len(lead):]
	return (1, lead,int(trail))
    else:
	return (0,"",0)
