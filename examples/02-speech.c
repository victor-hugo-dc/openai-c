#include "openai.h"

int main(void) {
    Client* openai = OpenAI("MY-API-KEY", "Organization", "Project");
    if (openai == NULL) {
        return EXIT_FAILURE;
    }

    transcribe(openai, "The quick brown fox jumps over the lazy dog.", "../output.mp3");

    destroyClient(openai);
    return EXIT_SUCCESS;
}
