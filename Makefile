dbuild:
	docker build -t pg-media-noauth . && docker run -d -p 5432:5432 --name pg-media-noauth pg-media-noauth

run:
	gcc main.c pkg/router/simple_router.c -o main && ./main

rpsql:
	gcc pkg/psql/psql_connection.c -o psql && ./psql
