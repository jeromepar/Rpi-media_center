#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mc_utils
import os
import commands
import encode_utf8

def interpret_cmd(gui, database, params, cmd):

	string=mc_utils.interpret_alias(cmd,params["aliases"])

	while '  ' in string:
		string=string.replace('  ',' ')

	str_cmd = string.strip()

	args=str_cmd.strip().split(' ')


	if "-hdmi" in args:
	    arg_audio_local = '-o hdmi '
	    args.remove('-hdmi')
	else:
	    arg_audio_local = ''

	if "-last" in args:
	    arg_select_last=1
	    args.remove("-last")
	else:
	    arg_select_last=0
	if "-en" in args:
	    arg_lang = 'en'
	    args.remove('-en')
	else:
	    arg_lang = 'fr'
	if "-force" in args:
	    arg_force_dl = 1
	    args.remove('-force')
	else:
	    arg_force_dl = 0
	if "-all" in args:
	    arg_all = 1
	    args.remove('-all')
	else:
	    arg_all = 0
	if "-exact" in args:
	    arg_exact = 1
	    args.remove('-exact')
	else:
	    arg_exact = 0
	if "-kb" in args:
	    fifo_on = 0
	    args.remove('-kb')
	else:
	    fifo_on = 1




	matching=mc_utils.match(args, "-*sec")
	if matching == '' or not mc_utils.is_int(matching):
	    start_position=''
	else:
	    start_position=' -l '+matching
	    args.remove('-*sec'.replace('*',matching))

	matching=mc_utils.match(args, "-*mn")
	if matching == '' or not mc_utils.is_int(matching):
	    start_position=''
	else:
	    start_position=' -l '+matching*60
	    args.remove('-*mn'.replace('*',matching))

	matching=mc_utils.match(args, "-*%")
	if matching == '' or not mc_utils.is_int(matching):
	    start_position=''
	else:
	    start_position='%'+matching
	    args.remove('-*%'.replace('*',matching))

	l_arg=len(args)
	cmd=args[0]

	if cmd == "play":
	    	if arg_exact:
		   if not '"' in args[1]:
			result = [args[1]]
		   else:
		        result=[mc_utils.match(str_cmd,'"*"')]
		else:
		    result = mc_utils.find_name(gui, args, database)

		if len(result)==0:
			mc_utils.print_txt(gui,  "Pas de résultats trouvés", 'error')
		else:
			if len(result)> 1:
				if arg_select_last:
					result=result.pop()
				else:
				    	str_cmd = str_cmd.replace(' '+args[-1],' ')
				        gui.mainFrame.body = mc_utils.choice(gui, str_cmd, result)
					return
			else:
				result=result[0]
			if result != '':
			        path = mc_utils.get_path(gui, result, database)

				#Gestion des sous-titre
				sub_path = mc_utils.get_subtitle_path(path)
				if os.path.exists(sub_path):
				    codec = mc_utils.get_subtitle_codec(sub_path)
				    if codec != "UTF-8":
					 mc_utils.print_txt(gui, "Conversion du fichier de sous-titre de "+codec+" en UTF-8 ...", 'info')
					 gui.mainLoop.draw_screen()
					 encode_utf8.convert_to_utf8(sub_path, codec)
					 gui.mainLoop.draw_screen()

				#Gestion de la position de départ
			    	if '%' in start_position:
				    percent=int(start_position[1:2])
				    time = mc_utils.get_video_duration(path)
				    start_position = " -l "+str(int(time*percent/100))

				mc_utils.play(path,params["play_options"]+' '+arg_audio_local +start_position,fifo_on)
	elif cmd == "info":
	    	if arg_exact:
		   if not '"' in args[1]:
			result = [args[1]]
		   else:
		        result=[mc_utils.match(str_cmd,'"*"')]
		else:
		    result = mc_utils.find_name(gui, args, database)

		if len(result)==0:
			mc_utils.print_txt(gui,  "Pas de résultats trouvés", 'error')
		else:
			if len(result)> 1:
				if arg_select_last:
					result=result.pop()
				else:
				    	str_cmd = str_cmd.replace(' '+args[-1],' ')
				        gui.mainFrame.body = mc_utils.choice(gui, str_cmd, result)
					return
			else:
				result=result[0]
			if result != '':
			    path = mc_utils.get_path(gui, result, database)
			    info = mc_utils.get_video_info(path)
			    mc_utils.print_txt(gui,  info ,'out')


	elif cmd == "stream":
		result = mc_utils.find_stream(gui, args, params["streams"])
		if len(result)==0:
		    mc_utils.print_txt(gui,  "Pas de résultats trouvés", 'error')
		else:
		    mc_utils.print_txt(gui, "Chargement du stream "+result,'info')
		    gui.mainLoop.draw_screen()
		    out = mc_utils.play_stream(result,params["play_options"])
		    gui.mainLoop.draw_screen()
	
	elif cmd == "update":
		mc_utils.print_txt(gui, "Mise à jour de la bibliothèque en cours ...",'info')
		gui.mainLoop.draw_screen()
		gui.database = mc_utils.generate_database(gui.params)
		mc_utils.print_txt(gui, "Done",'out')
	
	elif cmd == "torrents":
		mc_utils.print_txt(gui, "Etat des torrents:",'info')
		out = commands.getoutput("transmission-remote --auth=user:password --list | sed -e '1d;$d;s/^ *//'")
		mc_utils.print_txt(gui, out,'out')
	
	elif cmd == "sub":
	    	if arg_exact:
		   if not '"' in args[1]:
			result = [args[1]]
		   else:
		        result=[mc_utils.match(str_cmd,'"*"')]
		else:
		    result = mc_utils.find_name(gui,args, database)
		if len(result)==0:
			mc_utils.print_txt(gui,  "Pas de fichiers à ce nom trouvés", 'error')
		else:
			if len(result)>1:
				if arg_select_last:
					result=result.pop()
				elif(arg_all):
					mc_utils.print_txt(gui,  "Application multiple de la commande",'info')
					list_cmds=list()
					for e in result:
					    str_cmd = str_cmd.replace('-all','')
					    str_cmd = str_cmd.replace(' '+args[0]+' ',' ')
					    if len(args)>1:
						str_cmd = str_cmd.replace(' '+args[1]+' ',' ')
					    list_cmds.append(str_cmd+' "'+e+'" -exact')
					return list_cmds
				else:
				    	str_cmd = str_cmd.replace(' '+args[-1],' ')
				        gui.mainFrame.body = mc_utils.choice(gui, str_cmd, result)
					return
			else:
				result=result[0]

			if result != '':
			        path = mc_utils.get_path(gui, result, database)
				sub_path = mc_utils.get_subtitle_path(path)
				if arg_force_dl:
					os.system("rm "+mc_utils.adjust_path(sub_path))
				if os.path.exists(sub_path):
					mc_utils.print_txt(gui,  "Le sous titre "+
						result+".srt existe deja",'error')
				else:
					mc_utils.print_txt(gui,  "Recherche du sous-titre de : "
							+result,'info')
					gui.mainLoop.draw_screen()
					out=commands.getoutput("subliminal -l "+arg_lang+" -s -p opensubtitles -- "
							+mc_utils.adjust_path(path))
					if "No subtitles downloaded" in out:
					    mc_utils.print_txt(gui, "not found on opensubtitles, looking wider",'out')
					    gui.mainLoop.draw_screen()
					    out=commands.getoutput("subliminal -l "+arg_lang+' -s -- '
							    +mc_utils.adjust_path(path))
					gui.mainLoop.draw_screen()
					mc_utils.print_txt(gui, out,'out')
	elif cmd == "ls":
		result=mc_utils.list_files(gui,args,database)
		for r in result:
			mc_utils.print_txt(gui,  r,'out')
	elif cmd == "lspath":
		result=mc_utils.list_paths(gui,args,database)
		for r in result:
			mc_utils.print_txt(gui,  r,'out')
	elif cmd == "f":
		result=mc_utils.find_name(gui,args,database)
		for r in result:
			mc_utils.print_txt(gui,  r, 'out')
	elif cmd == "fp":
		result=mc_utils.find_paths(gui,args,database)
		for r in result:
			mc_utils.print_txt(gui,  r, 'out')
	elif cmd == "mem":
		mc_utils.print_txt(gui,  "Occupation des disques", 'info')
		out = commands.getoutput("df -kh | grep dev | grep G")
		mc_utils.print_txt(gui,  "Path            Total  Used  Free      Disk", 'out')
		mc_utils.print_txt(gui,  out, 'out')
	elif cmd == "help":
		mc_utils.print_txt(gui,  "\thelp\n", 'info')
		out = "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		out = out + "\t\thelp\t->\tdisplay this help\n"
		mc_utils.print_txt(gui,  out, 'out')

	elif cmd in params["custom_cmds"]:
		extern_cmd = params["custom_cmds"][cmd]
		extern_cmd = extern_cmd +' '+ str_cmd[len(args[0]):]
		mc_utils.print_txt(gui,  "Commande externe: "+extern_cmd, 'info')
    		gui.mainLoop.draw_screen()
		#out = commands.getoutput(extern_cmd)
		os.system(extern_cmd)
	else:
		mc_utils.print_txt(gui,  "commande inconnue:", 'error')
		mc_utils.print_txt(gui,  cmd, 'error')
