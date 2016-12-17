import socket

TCP_IP = '127.0.0.1'
TCP_PORT = 7777
BUFFER_SIZE = 1024
#MESSAGE = "GET /class/public/pages/sykes_webdesign/05_simple.html HTTP/1.1\r\nHost: csb.stanford.edu\r\n\r\n"
MESSAGE = "GET / HTTP/1.1\r\nHost: ya.ru\r\n\r\n"
all_data = []
for num in range(100):
	print "connection #", num
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((TCP_IP, TCP_PORT))
	s.send(MESSAGE)
	data = s.recv(BUFFER_SIZE)
	s.close()
	print "done"
	all_data += [data]
for i in range(1, len(all_data)):
	if (all_data[i] != all_data[i - 1]):
		print "answers for the same query are different:"
		print "-------------------------------------------------------------------------------------------"
		print all_data[i]
		print "-------------------------------------------------------------------------------------------"
		print all_data[i - 1]
		print "-------------------------------------------------------------------------------------------"
		exit(0)
print "Ok!"
