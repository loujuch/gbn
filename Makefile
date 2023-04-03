OBJ=main.o shell.o log.o config.o channel.o file_pdu_stream.o pdu.o receiver.o sender.o slide_window.o timer.o util.o
CC=g++ -std=c++11
LIB=-lpthread

host: ${OBJ}
	${CC} $^ -o $@ ${LIB}

main.o: main.cpp
	${CC} -c $< -o $@

%.o: %.cpp %.hpp
	${CC} -c $< -o $@

.PHONY:clean
clean:
	-rm -rf *.o host