#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    const char *name;
} User;

static int parse_id(const char *text)
{
    if (text == NULL) {
        return -1;
    }
    return atoi(text);
}

static void process_user(User *user)
{
    /* Intentionally crash: user may be NULL. */
    printf("user id=%d, name=%s\n", user->id, user->name);
}

static void handle_request(const char *id_text)
{
    int id = parse_id(id_text);
    User *user = NULL;

    if (id > 0) {
        user = (User *)malloc(sizeof(User));
        if (user == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        user->id = id;
        user->name = "demo_user";
    }

    process_user(user);
    free(user);
}

int main(void)
{
    puts("about to crash for coredump demo...");
    handle_request(NULL);
    return 0;
}
