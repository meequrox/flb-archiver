#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct MemoryStruct {
    char* memory;
    size_t size;
};

size_t write_memory_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "%s: not enough memory\n", __FUNCTION__);
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();

    const char* baseurl = "https://flareboard.ru/thread.php?id=1327";
    //    const char* baseurl = "https://flareboard.ru/thread.php?id=2666";
    struct MemoryStruct chunk = {malloc(1), 0};

    curl_easy_setopt(curl_handle, CURLOPT_URL, baseurl);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT,
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0");

    CURLcode response = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    if (response == CURLE_OK) {
        fprintf(stdout, "%lu bytes retrieved\n", (unsigned long)chunk.size);
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s", curl_easy_strerror(response));
    }

    fprintf(stdout, "%s\n", chunk.memory);
    free(chunk.memory);
    curl_global_cleanup();
    return 0;
}
