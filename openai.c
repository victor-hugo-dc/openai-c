#include "openai.h"
#include <cjson/cJSON.h>
#include <stddef.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Client* OpenAI(char* apiKey) {
    if (apiKey == NULL) {
        return NULL;
    }

    Client* result = (Client*) malloc(sizeof(Client));
    if (result == NULL) {
        return NULL;
    }

    ClientOptions* options = (ClientOptions*) malloc(sizeof(ClientOptions));
    if (options == NULL) {
        free(result);
        return NULL;
    }
    

    // Allocate memory and copy the strings
    result->apiKey = strdup(apiKey);
    result->organization = NULL;
    result->project = NULL;

    options->apiKey = strdup(apiKey);
    options->organization = NULL;
    options->project = NULL;
    options->baseURL = "https://api.openai.com/";
    options->timeout = 600000;
    options->maxRetries = 2;
    options->dangerouslyAllowBrowser = false;

    result->_options = options;

    result->chat_model = GPT_3_5_TURBO;
    result->temperature = 0.8;

    result->speech_model = TTS_1;
    result->voice = ALLOY;

    result->verbose = false;

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

const char* getChatModelName(enum GPT model) {
    switch (model) {
        case GPT_3_5_TURBO_1106:
            return "gpt-3.5-turbo-1106";

        case GPT_3_5_TURBO_16k:
            return "gpt-3.5-turbo-16k";

        case GPT_4:
            return "gpt-4";

        case GPT_4_TURBO_2024_04_09:
            return "gpt-4-turbo-2024-04-09";

        case GPT_4_0613:
            return "gpt-4-0613";
        
        case GPT_4_TURBO:
            return "gpt-4-turbo";

        case GPT_4_VISION_PREVIEW:
            return "gpt-4-vision-preview";

        case GPT_3_5_TURBO_INSTRUCT_0914:
            return "gpt-3.5-turbo-instruct-0914";

        case GPT_3_5_TURBO_INSTRUCT:
            return "gpt-3.5-turbo-instruct";

        case GPT_4_1106_VISION_PREVIEW:
            return "gpt-4-1106-vision-preview";

        case GPT_4_0125_PREVIEW:
            return "gpt-4-0125-preview";

        case GPT_4_TURBO_PREVIEW:
            return "gpt-4-turbo-preview";

        case GPT_3_5_TURBO_0125:
            return "gpt-3.5-turbo-0125";

        case GPT_3_5_TURBO:
            return "gpt-3.5-turbo";
        
        case GPT_3_5_TURBO_0613:
            return "gpt-3.5-turbo-0613";

        case GPT_4_1106_PREVIEW:
            return "gpt-4-1106-preview";

        case GPT_3_5_TURBO_16k_0613:
            return "gpt-3.5-turbo-16k-0613";

        default:
            return "gpt-3.5-turbo";
    }
}

Completion* chat(Client* openai, const char* messages) {
    
    Completion* response = NULL;
    char* url = NULL;
    char* authHeader = NULL;
    char* requestBody = NULL;
    cJSON *root = NULL;

    String s;
    initString(&s);

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
    const char* model = getChatModelName(openai->chat_model);
    length = snprintf(NULL, 0, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}], \"temperature\": %.1f}", model, messages, openai->temperature);
    requestBody = (char*) malloc(length + 1);
    if (requestBody == NULL) {
        goto Exit;
    }
    snprintf(requestBody, length + 1, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}], \"temperature\": %.1f}", model, messages, openai->temperature);

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
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        goto Exit;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    response = (Completion*) malloc(sizeof(Completion));
    if (response == NULL) {
        goto Exit;
    }

    root = cJSON_Parse(s.ptr);

    cJSON* errorJson = cJSON_GetObjectItemCaseSensitive(root, "error");

    response->error = NULL;
    // idk if we have to check if its NULL or is cJSON_isNULL
    if (errorJson != NULL) {

        char* message = cJSON_GetObjectItemCaseSensitive(errorJson, "message")->valuestring;
        char* type = cJSON_GetObjectItemCaseSensitive(errorJson, "type")->valuestring;
        char* param = cJSON_GetObjectItemCaseSensitive(errorJson, "param")->valuestring; // this can be NULL so we might have to change
        char* code = cJSON_GetObjectItemCaseSensitive(errorJson, "code")->valuestring;

        response->error = (Error*) malloc(sizeof(Error));
        response->error->message = strdup(message);
        response->error->type = strdup(type);
        response->error->param = NULL; // this may have to change later
        response->error->code = strdup(code);

    }

    char *id = cJSON_GetObjectItemCaseSensitive(root, "id")->valuestring;
    char *object = cJSON_GetObjectItemCaseSensitive(root, "object")->valuestring;
    int created = cJSON_GetObjectItemCaseSensitive(root, "created")->valueint;
    const char *model_string = cJSON_GetObjectItemCaseSensitive(root, "model")->valuestring;

    cJSON *system_fingerprint = cJSON_GetObjectItemCaseSensitive(root, "system_fingerprint");
    if (system_fingerprint != NULL) {
        if (cJSON_IsString(system_fingerprint)) {
            response->system_fingerprint = strdup(system_fingerprint->valuestring);
        } else if (cJSON_IsNull(system_fingerprint)) {
            response->system_fingerprint = strdup("null");
        }
    }

    // TODO: Robustly check for NULL
    response->id = strdup(id);
    response->object = strdup(object);
    response->created = created;
    response->model = strdup(model_string);

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

void destroyCompletion(Completion* response) {
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
        free(response->error);

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

ModelList* models(Client* openai) {
    ModelList* response = NULL;
    char* url = NULL;
    char* authHeader = NULL;
    cJSON *root = NULL;

    String s;
    initString(&s);

    // Create URL Endpoint
    int length = snprintf(NULL, 0, "%s/%s", openai->_options->baseURL, "v1/models");
    url = (char*) malloc(length + 1);
    if (url == NULL) {
        goto Exit;
    }
    snprintf(url, length + 1, "%s/%s", openai->_options->baseURL, "v1/models");

    // Create Authorization Header
    length = snprintf(NULL, 0, "Authorization: Bearer %s", openai->apiKey);
    authHeader = (char*) malloc(length + 1);
    if (authHeader == NULL) {
        goto Exit;
    }
    snprintf(authHeader, length + 1, "Authorization: Bearer %s", openai->apiKey);

    CURL* curl = curl_easy_init();
    if (!curl) {
        goto Exit;
    }

    // Set up CURL request
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, authHeader);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        goto Exit;
    }

    response = (ModelList*) malloc(sizeof(ModelList));
    if (response == NULL) {
        goto Exit;
    }
    
    root = cJSON_Parse(s.ptr);

    char* object = cJSON_GetObjectItemCaseSensitive(root, "object")->valuestring;
    response->object = strdup(object);

    cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
    int model_count = cJSON_GetArraySize(data);

    response->data = (Model**) malloc(model_count * sizeof(Model*));

    for (size_t i = 0; i < model_count; i++) {
        response->data[i] = (Model*) malloc(sizeof(Model));
        cJSON* model = cJSON_GetArrayItem(data, i);

        char* id = cJSON_GetObjectItemCaseSensitive(model, "id")->valuestring;
        char* individual_object = cJSON_GetObjectItemCaseSensitive(model, "object")->valuestring;
        int created = cJSON_GetObjectItemCaseSensitive(model, "created")->valueint;
        char* owned_by = cJSON_GetObjectItemCaseSensitive(model, "owned_by")->valuestring;

        response->data[i]->id = strdup(id);
        response->data[i]->object = strdup(individual_object);
        response->data[i]->created = created;
        response->data[i]->owned_by = strdup(owned_by);

    }

    response->model_count = model_count;

    // I believe this goes here
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

Exit:
    free(url);
    free(authHeader);
    free(s.ptr);
    cJSON_Delete(root);
    return response;
}

void destroyModels(ModelList* models) {
    free(models->object);
    for (size_t i = 0; i < models->model_count; i++) {
        free(models->data[i]->id);
        free(models->data[i]->object);
        free(models->data[i]->owned_by);
        free(models->data[i]);
    }

    free(models->data);
    free(models);
}

const char* getSpeechModel(enum TTS model) {
    switch (model) {
        case TTS_1:
            return "tts-1";
        
        case TTS_1_HD:
            return "tts-1-hd";

        default:
            return "tts-1";
    }
}

const char* getVoice(enum VOICE voice) {
    switch (voice) {
        case ALLOY:
            return "alloy";

        case ECHO:
            return "echo";
        
        case FABLE:
            return "fable";
        
        case ONYX:
            return "onyx";
        
        case NOVA:
            return "nova";
        
        case SHIMMER:
            return "shimmer";
        
        default:
            return "alloy";
    }
}

void generate_speech(Client* openai, const char* message, const char* filename) {

    char* url = NULL;
    char* authHeader = NULL;
    char* requestBody = NULL;
    CURLcode res;

    // Create URL Endpoint
    int length = snprintf(NULL, 0, "%s/%s", openai->_options->baseURL, "v1/audio/speech");
    url = (char*) malloc(length + 1);
    if (url == NULL) {
        goto Exit;
    }
    snprintf(url, length + 1, "%s/%s", openai->_options->baseURL, "v1/audio/speech");
    
    // Create Authorization Header
    length = snprintf(NULL, 0, "Authorization: Bearer %s", openai->apiKey);
    authHeader = (char*) malloc(length + 1);
    if (authHeader == NULL) {
        goto Exit;
    }
    snprintf(authHeader, length + 1, "Authorization: Bearer %s", openai->apiKey);

    // Create request body
    const char* model = getSpeechModel(openai->speech_model);
    const char* voice = getVoice(openai->voice);
    length = snprintf(NULL, 0, "{\"model\": \"%s\", \"input\": \"%s\", \"voice\": \"%s\"}", model, message, voice);
    requestBody = (char*) malloc(length + 1);
    if (requestBody == NULL) {
        goto Exit;
    }
    snprintf(requestBody, length + 1, "{\"model\": \"%s\", \"input\": \"%s\", \"voice\": \"%s\"}", model, message, voice);

    CURL* curl = curl_easy_init();
    if (!curl) {
        goto Exit;
    }

    // Set up CURL request
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, authHeader);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody);

    // Here have some verification that the filename is of type .mp3
    FILE *output_file = fopen(filename, "wb");
    if (!output_file) {
        curl_easy_cleanup(curl);
        goto Exit;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, output_file);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Cleanup
    fclose(output_file);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

Exit:
    free(url);
    free(authHeader);
    free(requestBody);
}

int valid_filename(const char *filename) {
    const char *valid_extensions[] = {"flac", "mp3", "mp4", "mpeg", "mpga", "m4a", "ogg", "wav", "webm"};
    size_t num_extensions = sizeof(valid_extensions) / sizeof(valid_extensions[0]);
    
    const char *dot = strrchr(filename, '.');
    if (dot == NULL) {
        return 0;
    }
    
    const char *extension = dot + 1;
    for (size_t i = 0; i < num_extensions; i++) {
        if (strcmp(extension, valid_extensions[i]) == 0) {
            return 1;
        }
    }
    
    return 0;
}

Transcription* transcribe(Client* openai, const char* filepath) {
    Transcription* response = NULL;
    char* url = NULL;
    char* authHeader = NULL;
    char* requestBody = NULL;
    CURLcode res;
    cJSON* root = NULL;

    String s;
    initString(&s);

    if (valid_filename(filepath) == 0) {
        fprintf(stderr, "Invalid filetype.\n");
        goto Exit;
    }

    // Create URL Endpoint
    int length = snprintf(NULL, 0, "%s/%s", openai->_options->baseURL, "v1/audio/transcriptions");
    url = (char*) malloc(length + 1);
    if (url == NULL) {
        goto Exit;
    }
    snprintf(url, length + 1, "%s/%s", openai->_options->baseURL, "v1/audio/transcriptions");

    length = snprintf(NULL, 0, "Authorization: Bearer %s", openai->apiKey);
    authHeader = (char*) malloc(length + 1);
    if (authHeader == NULL) {
        goto Exit;
    }
    snprintf(authHeader, length + 1, "Authorization: Bearer %s", openai->apiKey);

    CURL* curl = curl_easy_init();
    if (!curl) {
        goto Exit;
    }

    // Set up CURL request
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, authHeader);
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;

    curl_formadd(&post, &last, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filepath, CURLFORM_END);
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "model", CURLFORM_COPYCONTENTS, "whisper-1", CURLFORM_END);

    if (openai->verbose) {
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "timestamp_granularities[]", CURLFORM_COPYCONTENTS, "word", CURLFORM_END);
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "response_format", CURLFORM_COPYCONTENTS, "verbose_json", CURLFORM_END);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_formfree(post);
        curl_global_cleanup();
        goto Exit;
    }

    root = cJSON_Parse(s.ptr);

    response = (Transcription*) malloc(sizeof(Transcription));

    cJSON* text_object = cJSON_GetObjectItemCaseSensitive(root, "text");
    if (text_object == NULL) {
        cJSON* errorJson = cJSON_GetObjectItemCaseSensitive(root, "error");

        char* message = cJSON_GetObjectItemCaseSensitive(errorJson, "message")->valuestring;
        char* type = cJSON_GetObjectItemCaseSensitive(errorJson, "type")->valuestring;
        
        char* param;
        cJSON* parameters = cJSON_GetObjectItemCaseSensitive(errorJson, "param");
        if (cJSON_IsNull(parameters)) {
            param = "null";

        } else {
            param = parameters->valuestring;

        }

        char* code = cJSON_GetObjectItemCaseSensitive(errorJson, "code")->valuestring;

        response->error = (Error*) malloc(sizeof(Error));
        response->error->message = strdup(message);
        response->error->type = strdup(type);
        response->error->param = strdup(param); // this may have to change later
        response->error->code = strdup(code);
        response->text = NULL;

        response->duration = 0;
        response->language = NULL;
        response->task = NULL;
        response->words = NULL;
        response->word_count = 0;
        
    } else {
        char* text = text_object->valuestring;
        response->text = strdup(text);
        response->error = NULL;

        if (openai->verbose) {
            char* task = cJSON_GetObjectItemCaseSensitive(root, "task")->valuestring;
            char* language = cJSON_GetObjectItemCaseSensitive(root, "language")->valuestring;
            double duration = cJSON_GetObjectItemCaseSensitive(root, "duration")->valuedouble;
            cJSON* wordsJSON = cJSON_GetObjectItemCaseSensitive(root, "words");

            response->task = strdup(task);
            response->language = strdup(language);
            response->duration = duration;

            int word_count = cJSON_GetArraySize(wordsJSON);
            response->words = (Word**) malloc(word_count * sizeof(Word*));

            for (size_t i = 0; i < word_count; i++) {
                response->words[i] = (Word*) malloc(sizeof(Word));

                cJSON* object = cJSON_GetArrayItem(wordsJSON, i);
                char* word = cJSON_GetObjectItemCaseSensitive(object, "word")->valuestring;
                double start = cJSON_GetObjectItemCaseSensitive(object, "start")->valuedouble;
                double end = cJSON_GetObjectItemCaseSensitive(object, "end")->valuedouble;

                response->words[i]->word = strdup(word);
                response->words[i]->start = start;
                response->words[i]->end = end;

            }

            response->word_count = word_count;

        } else {
            response->duration = 0;
            response->language = NULL;
            response->task = NULL;
            response->words = NULL;
            response->word_count = 0;

        }
    }

    curl_easy_cleanup(curl);
    curl_formfree(post);
    curl_global_cleanup();

Exit:
    free(url);
    free(authHeader);
    free(requestBody);
    free(s.ptr);
    cJSON_Delete(root);

    return response;
}

void destroyTranscription(Transcription* transcription) {
    if (transcription->error != NULL) {
        free(transcription->error->message);
        free(transcription->error->type);
        free(transcription->error->param);
        free(transcription->error->code);
        free(transcription->error);

    }

    if (transcription->words != NULL) {
        for (size_t i = 0; i < transcription->word_count; i++) {
            free(transcription->words[i]->word);
            free(transcription->words[i]);
        }
        
        free(transcription->words); 
    }
    
    free(transcription->task);
    free(transcription->language);

    free(transcription->text);
    free(transcription);
}