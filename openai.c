#include "openai.h"
#include "cJSON.h"
#include <stddef.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Client* OpenAI(char* apiKey, char* organization, char* project) {
    // API key must be defined
    if (apiKey == NULL) {
        return NULL;
    }

    Client* result = (Client*) malloc(sizeof(Client));

    // Memory allocation failed
    if (result == NULL) {
        return NULL;
    }

    ClientOptions* options = (ClientOptions*) malloc(sizeof(ClientOptions));
    
    // Memory allocation failed
    if (options == NULL) {
        free(result);
        return NULL;
    }
    

    // Allocate memory and copy the strings
    result->apiKey = strdup(apiKey);
    result->organization = strdup(organization);
    result->project = strdup(project);

    // TODO: Allow user to pass these in as parameters
    options->apiKey = strdup(apiKey);
    options->organization = strdup(organization);
    options->project = strdup(project);
    options->baseURL = "https://api.openai.com/";
    options->timeout = 600000;
    options->maxRetries = 2;
    options->dangerouslyAllowBrowser = false;

    result->_options = options;

    return result;
}

void destroyClient(Client *object) {
    if (object == NULL) {
        return;
    }

    free(object->apiKey);
    free(object->organization);
    free(object->project);

    // Free the client options
    free(object->_options->apiKey);
    free(object->_options->organization);
    free(object->_options->project);
    free(object->_options);

    free(object);
}

void initString(String *string) {
    string->length = 0;
    string->ptr = malloc(string->length + 1);
    if (string->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    string->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, String *s) {
    size_t length = s->length + size * nmemb;
    s->ptr = realloc(s->ptr, length + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->length, ptr, size * nmemb);
    s->ptr[length] = '\0';
    s->length = length;
    return size * nmemb;
}

void destroyChoicesList(Choices** choicesList, int count) {
    if (choicesList == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        destroyChoices(choicesList[i]);
    }

    free(choicesList);
}


void destroyMessage(Message* message) {
    if (message == NULL) {
        return;
    }

    if (message->content) {
        free(message->content);
    }
    
    free(message->role);
    free(message);
}

Response* chat(Client* openai, const char* model, const char* messages, float temperature) {
    
    Response* response = NULL;
    char* url = NULL;
    char* authHeader = NULL;
    char* requestBody = NULL;
    cJSON *root = NULL;

    // Create URL Endpoint
    int length = snprintf(NULL, 0, "%s/%s", openai->_options->baseURL, "v1/chat/completions");
    url = (char*) malloc(length + 1);
    if (url == NULL) {
        goto Exit;
    }
    snprintf(url, length + 1, "%s/%s", openai->_options->baseURL, "v1/chat/completions");
    
    // Create Authorization Header
    length = snprintf(NULL, 0, "Authorization: Bearer %s", openai->apiKey);
    authHeader = (char*) malloc(length + 1);
    if (authHeader == NULL) {
        goto Exit;
    }
    snprintf(authHeader, length + 1, "Authorization: Bearer %s", openai->apiKey);

    // Create request body
    length = snprintf(NULL, 0, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}], \"temperature\": %.1f}", model, messages, temperature);
    requestBody = (char*) malloc(length + 1);
    if (requestBody == NULL) {
        goto Exit;
    }
    snprintf(requestBody, length + 1, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}], \"temperature\": %.1f}", model, messages, temperature);
    
    String s;
    initString(&s);

    CURL* curl = curl_easy_init();
    if (!curl) {
        goto Exit;
    }

    // Set up CURL request
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    response = (Response*) malloc(sizeof(Response));
    if (response == NULL) {
        goto Exit;
    }

    root = cJSON_Parse(s.ptr);

    cJSON* errorJson = cJSON_GetObjectItemCaseSensitive(root, "error");

    response->error = NULL;
    if (errorJson != NULL) {

        char* message = cJSON_GetObjectItemCaseSensitive(errorJson, "message")->valuestring;
        char* type = cJSON_GetObjectItemCaseSensitive(errorJson, "type")->valuestring;
        char* param = cJSON_GetObjectItemCaseSensitive(errorJson, "param")->valuestring; // this can be NULL so we might have to change
        char* code = cJSON_GetObjectItemCaseSensitive(errorJson, "code")->valuestring;

        response->error = (Error*) malloc(sizeof(Error));
        response->error->message = strdup(message);
        response->error->type = strdup(param);
        response->error->param = NULL; // this may have to change later
        response->error->code = strdup(code);

    }

    char *id = cJSON_GetObjectItemCaseSensitive(root, "id")->valuestring;
    char *object = cJSON_GetObjectItemCaseSensitive(root, "object")->valuestring;
    int created = cJSON_GetObjectItemCaseSensitive(root, "created")->valueint;
    const char *model_string = cJSON_GetObjectItemCaseSensitive(root, "model")->valuestring;
    char *system_fingerprint = cJSON_GetObjectItemCaseSensitive(root, "system_fingerprint")->valuestring;

    // TODO: Robustly check for NULL
    response->id = strdup(id);
    response->object = strdup(object);
    response->created = created;
    response->model = strdup(model_string);
    response->system_fingerprint = strdup(system_fingerprint);

    cJSON* usageJSON = cJSON_GetObjectItemCaseSensitive(root, "usage");
    response->usage = (Usage*) malloc(sizeof(Usage));
    response->usage->prompt_tokens = cJSON_GetObjectItemCaseSensitive(usageJSON, "prompt_tokens")->valueint;
    response->usage->completion_tokens = cJSON_GetObjectItemCaseSensitive(usageJSON, "completion_tokens")->valueint;
    response->usage->total_tokens = cJSON_GetObjectItemCaseSensitive(usageJSON, "total_tokens")->valueint;

    cJSON *choicesJSON = cJSON_GetObjectItemCaseSensitive(root, "choices");
    int choiceCount = cJSON_GetArraySize(choicesJSON);

    response->choices = (Choices**) malloc(choiceCount * sizeof(Choices*));
    if (!response->choices) {
        free(response);
        goto Exit;
    }

    for (size_t i = 0; i < choiceCount; i++) {
        // TODO: Check for Malloc failure
        response->choices[i] = (Choices*) malloc(sizeof(Choices));
        cJSON* choice = cJSON_GetArrayItem(choicesJSON, i);

        int index = cJSON_GetObjectItemCaseSensitive(choice, "index")->valueint;

        cJSON *logprobsObject = cJSON_GetObjectItemCaseSensitive(choice, "logprobs");
        if (logprobsObject != NULL) {
            if (cJSON_IsString(logprobsObject)) {
                response->choices[i]->logprobs = strdup(logprobsObject->valuestring);
            } else if (cJSON_IsNull(logprobsObject)) {
                response->choices[i]->logprobs = NULL; // or set it to a default value
            }
        }

        char* finish_reason = cJSON_GetObjectItemCaseSensitive(choice, "finish_reason")->valuestring;

        response->choices[i]->index = index;
        response->choices[i]->finish_reason = strdup(finish_reason);

        cJSON* messageJSON = cJSON_GetObjectItemCaseSensitive(choice, "message");
        response->choices[i]->message = (Message*) malloc(sizeof(Message));

        cJSON *messageObject = cJSON_GetObjectItemCaseSensitive(messageJSON, "role");
        response->choices[i]->message->role = strdup(messageObject->valuestring);

        cJSON *contentObject = cJSON_GetObjectItemCaseSensitive(messageJSON, "content");
        response->choices[i]->message->content = strdup(contentObject->valuestring);

    }

    response->choices_count = choiceCount;

Exit:
    free(url);
    free(authHeader);
    free(requestBody);
    free(s.ptr);
    cJSON_Delete(root);

    return response;
}

void destroyResponse(Response* response) {
    if (response == NULL) {
        return;
    }
    // Free allocated memory for each field
    free(response->id);
    free(response->object);
    free(response->model);
    free(response->system_fingerprint);
    if (response->choices != NULL) {
        for (int i = 0; i < response->choices_count; i++) {
            destroyChoices(response->choices[i]);
        }
        free(response->choices);
    }

    if (response->error != NULL) {
        free(response->error->message);
        free(response->error->type);
        free(response->error->param);
        free(response->error->code);

    }

    // Free the response object itself
    free(response);
}

void destroyChoices(Choices* choice) {
    if (choice == NULL) {
        return;
    }

    if (choice->message != NULL) {
        free(choice->message->content);
        free(choice->message->role);
        free(choice->message);
    }

    if (choice->logprobs != NULL) {
        free(choice->logprobs);
    }
    free(choice->finish_reason);
}