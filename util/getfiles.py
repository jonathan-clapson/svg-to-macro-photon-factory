#!/bin/python
import argparse, os, shutil

args = None

def main():
#parse arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('-u', '--user', dest='user_name', required=True, help='the user to get files for')
	parser.add_argument('-s', '--start-time', dest='start_time', type=int, default=0, help='minimum timestamp allowed')
	parser.add_argument('-e', '--end-time', dest='end_time', type=int, default=None, help='maximum timestamp allowed')
	parser.add_argument('-b', '--base-dir', dest='base_dir', default='/srv/http/svg-files', help='path to root svg storage directory')
	parser.add_argument('dest_dir', help='place to store files')
	args = parser.parse_args()
	
	#let the user know what we're doing
	print ('Searching directory: %s' % args.base_dir)
	print ('Output directory: %s' % args.dest_dir)
	print ('Retrieving files for user: %s' % args.user_name)
	print ('Using start time: %d' % args.start_time)	
	if args.end_time is None:
		print('Using end time: No limit')
	else:
		print('Using end time: %d' % args.end_time)
	
	#create the search path
	search_dir = args.base_dir + '/' + args.user_name

	#create an empty dictionary to hold the most up to data files
	file_dict = dict()

	#find most up to date files in search path
	file_list = os.listdir(search_dir)
	for file_name in file_list:
		#seperate out the file name and timestamp. Check the extension is correct
		#filename-timestamp.extension
		offset = file_name.rfind('-')
		file_name_str = file_name[:offset]		
		timestamp = file_name[offset+1:]
		offset = timestamp.find('.')
		extension = timestamp[offset+1:]
		timestamp = int(timestamp[:offset])
		
		#check file extension is svg (just in case)
		if extension != "svg":
			continue
		
		#check we don't exceed the lower limit
		if timestamp < args.start_time:
			continue
			
		#check we don't exceed the upper limit (if its set)
		if args.end_time is not None:
			if timestamp > args.end_time:
				continue
		
		if file_name_str in file_dict:
			if file_dict[file_name_str] < timestamp:
				file_dict[file_name_str] = timestamp
		else:
			file_dict[file_name_str] = timestamp
	
	#copy most up to date files to dest
	for key in file_dict:
		file_name = key+'-'+str(file_dict[key])+'.svg'
		in_file_path = search_dir + '/' + file_name
		out_file_path = args.dest_dir + '/' + file_name
		
		shutil.copyfile(in_file_path, out_file_path)
		#for security if the following is used, ensure the path to the binary is absolute, and make sure permissions are entirely read only
		#print ('converting file: ' + out_file_path)
		#args = ('svg-macro-convert', out_file_path)
		#popen = subprocess.Popen(args, stdout=subprocess.PIPE)
		#popen.wait()
		#output = popen.stdout.read()
		#print output
	
	print ("Saved: ")
	print (file_dict)
	
if __name__ == "__main__":
	main()
