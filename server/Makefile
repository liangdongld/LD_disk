SRC:=$(wildcard *.c)
thread_pool_server:$(SRC)
	gcc $^ -o ../upFile/$@ -lpthread -lcrypt -lmysqlclient
clean:
	rm thread_pool_server
