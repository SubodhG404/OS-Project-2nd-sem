TARGET = build/producer_consumer
SOURCES = main.cpp config.cpp shared.cpp semaphore.cpp visualization.cpp producer_consumer.cpp simulation_controller.cpp

$(TARGET): $(SOURCES) CMakeLists.txt
	cmake -S . -B build
	cmake --build build

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build
