all: frontend backend

frontend: frontend.c frontend.h
	gcc -o frontend frontend.c

backend: backend.c backend.h
	gcc -o backend backend.c users_lib.o -pthread
		
clean:
	rm frontend backend
