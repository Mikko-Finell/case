CC := clang++
CPPFLAGS := -std=c++14 -O3
LDFLAGS := -lsfml-system -lsfml-window -lsfml-graphics -lpthread

all: install demo
	
install: 
	install -v -D ./*.hpp --target-directory /usr/local/include/CASE

demo: $(patsubst demo/%.cpp, demo/%.out, $(wildcard demo/*.cpp))

demo/%.out: demo/%.cpp Makefile
	$(CC) $< -o $@ $(CPPFLAGS) $(LDFLAGS) -DCASE_DETERMINISTIC

clean:
	rm -rf $(wildcard demo/*.out)
