#include <stdio.h>
#include <stdlib.h>
#include "openai.h"

int main(void) {
    OpenAI* openai = createOpenAI("OPENAIKEY", "Personal", "Default Project");
    Response* response = chat(openai, "gpt-3.5-turbo-0125", "Hello world", 0.7);

    printf("%s\n", response->id);
    printf("%s\n", response->object);
    printf("%s\n", response->model);
    printf("%s\n", response->message_content);
    printf("%d\n", response->prompt_tokens);
    printf("%s\n", response->system_fingerprint);

    destroyResponse(response);
    return EXIT_SUCCESS;
}

