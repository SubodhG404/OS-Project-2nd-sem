#include "config.h"
#include "shared.h"
#include "visualization.h"
#include "producer_consumer.h"

int main() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 0;
    }

    run_visualization();
    return 0;
}
