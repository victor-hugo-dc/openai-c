#include "openai.h"
#include "cJSON.h"
#include <stddef.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OpenAI* createOpenAI(char* apiKey, char* organization, char* project) {
    // API key must be defined
    if (apiKey == NULL) {
        return NULL;
    }

    OpenAI* result = (OpenAI*)malloc(sizeof(OpenAI));

    // Memory allocation failed
    if (result == NULL) {
        return NULL;
    }

    ClientOptions* options = (ClientOptions*)malloc(sizeof(ClientOptions));
    
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

void destroyOpenAI(OpenAI *object) {
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

Response* chat(OpenAI* openai, const char* model, const char* messages, float temperature) {
    CURL* curl;
    CURLcode res;

    // Create URL endpoint
    int length = snprintf(NULL, 0, "%s/%s", openai->_options->baseURL, "v1/chat/completions");
    char* url = (char*) malloc(length + 1);
    if (url == NULL) {
        return NULL;
    }
    snprintf(url, length + 1, "%s/%s", openai->_options->baseURL, "v1/chat/completions");

    // Create Authorization Header
    length = snprintf(NULL, 0, "Authorization: Bearer %s", openai->apiKey);
    char* authHeader = (char*) malloc(length + 1);
    if (authHeader == NULL) {
        free(url);
        return NULL;
    }
    snprintf(authHeader, length + 1, "Authorization: Bearer %s", openai->apiKey);

    // Create request body
    length = snprintf(NULL, 0, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}], \"temperature\": %.1f}", model, messages, temperature);
    char* requestBody = (char*) malloc(length + 1);
    if (requestBody == NULL) {
        free(url);
        free(authHeader);
        return NULL;
    }
    snprintf(requestBody, length + 1, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}], \"temperature\": %.1f}", model, messages, temperature);
    
    String s;
    initString(&s);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, authHeader);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);
       
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        Response* response = (Response*) malloc(sizeof(Response));
        if (response == NULL) {
            free(url);
            free(requestBody);
            free(authHeader);
            return NULL;
        }
        printf("%s\n", s.ptr);

        cJSON *root = cJSON_Parse(s.ptr);
        const char *id = cJSON_GetObjectItemCaseSensitive(root, "id")->valuestring;
        const char *object = cJSON_GetObjectItemCaseSensitive(root, "object")->valuestring;
        int created = cJSON_GetObjectItemCaseSensitive(root, "created")->valueint;
        const char *model = cJSON_GetObjectItemCaseSensitive(root, "model")->valuestring;
        const char *finish_reason = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(root, "choices"), 0), "finish_reason")->valuestring;
        int prompt_tokens = cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(root, "usage"), "prompt_tokens")->valueint;
        int completion_tokens = cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(root, "usage"), "completion_tokens")->valueint;
        int total_tokens = cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(root, "usage"), "total_tokens")->valueint;
        const char *system_fingerprint = cJSON_GetObjectItemCaseSensitive(root, "system_fingerprint")->valuestring;

        response->id = id;
        response->object = object;
        response->created = created;
        response->model = model;
        response->finish_reason = finish_reason;
        response->completion_tokens = completion_tokens;
        response->total_tokens = total_tokens;
        response->system_fingerprint = system_fingerprint;
        free(s.ptr);
        return response;
    }

    free(url);
    free(requestBody);
    free(authHeader);
    return NULL;

}

void destroyResponse(Response* response) {
    if (response == NULL) {
        return;
    }
    // Free allocated memory for each field
    free(response->id);
    free(response->object);
    free(response->model);
    free(response->message_role);
    free(response->message_content);
    free(response->finish_reason);
    free(response->system_fingerprint);

    // Free the response object itself
    free(response);
}