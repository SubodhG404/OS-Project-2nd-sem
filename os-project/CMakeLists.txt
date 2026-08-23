cmake_minimum_required(VERSION 3.22)
project(producer_consumer_sfml LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SFML_STATIC_LIBRARIES ON)
find_package(SFML 3 REQUIRED COMPONENTS Graphics)

add_executable(producer_consumer
    main.cpp config.cpp shared.cpp semaphore.cpp visualization.cpp producer_consumer.cpp simulation_controller.cpp)
target_link_libraries(producer_consumer PRIVATE SFML::Graphics)
