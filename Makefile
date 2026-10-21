default:
	g++ src/main.cpp -o Game -Iinclude -Llib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	./Game