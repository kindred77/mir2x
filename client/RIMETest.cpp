#include <rime_api.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>

void on_message([[maybe_unused]] void* context_object,
                RimeSessionId session_id,
                const char* message_type,
                const char* message_value) {
    printf("message: [%zu] [%s] %s\n", session_id, message_type, message_value);
    RimeApi* rime = rime_get_api();
    if (RIME_API_AVAILABLE(rime, get_state_label) &&
        !strcmp(message_type, "option")) {
        Bool state = message_value[0] != '!';
        const char* option_name = message_value + !state;
        const char* state_label =
            rime->get_state_label(session_id, option_name, state);
        if (state_label) {
            printf("updated option: %s = %d // %s\n", option_name, state,
                   state_label);
        }
    }
}

RimeSessionId ensure_session(RimeApi* rime) {
    RimeSessionId id = rime->create_session();
    if (!id) {
        fprintf(stderr, "Error creating rime session.\n");
    }
    return id;
}

void print_status(RimeStatus* status) {
    printf("schema: %s / %s\n", status->schema_id, status->schema_name);
    printf("status: ");
    if (status->is_disabled)
        printf("disabled ");
    if (status->is_composing)
        printf("composing ");
    if (status->is_ascii_mode)
        printf("ascii ");
    if (status->is_full_shape)
        printf("full_shape ");
    if (status->is_simplified)
        printf("simplified ");
    printf("\n");
}

void print_composition(RimeComposition* composition) {
    const char* preedit = composition->preedit;
    if (!preedit)
        return;
    size_t len = strlen(preedit);
    size_t start = composition->sel_start;
    size_t end = composition->sel_end;
    size_t cursor = composition->cursor_pos;
    for (size_t i = 0; i <= len; ++i) {
        if (start < end) {
            if (i == start) {
                putchar('[');
            } else if (i == end) {
                putchar(']');
            }
        }
        if (i == cursor)
            putchar('|');
        if (i < len)
            putchar(preedit[i]);
    }
    printf("\n");
}

void print_menu(RimeMenu* menu) {
    if (menu->num_candidates == 0)
        return;
    printf("page: %d%c (of size %d)\n", menu->page_no + 1,
           menu->is_last_page ? '$' : ' ', menu->page_size);
    for (int i = 0; i < menu->num_candidates; ++i) {
        bool highlighted = i == menu->highlighted_candidate_index;
        printf("%d. %c%s%c%s\n", i + 1, highlighted ? '[' : ' ',
               menu->candidates[i].text, highlighted ? ']' : ' ',
               menu->candidates[i].comment ? menu->candidates[i].comment : "");
    }
}

void print_context(RimeContext* context) {
    if (context->composition.length > 0 || context->menu.num_candidates > 0) {
        print_composition(&context->composition);
    } else {
        printf("(not composing)\n");
    }
    print_menu(&context->menu);
}

void print(RimeSessionId session_id) {
    RimeApi* rime = rime_get_api();

    RIME_STRUCT(RimeCommit, commit);
    RIME_STRUCT(RimeStatus, status);
    RIME_STRUCT(RimeContext, context);

    if (rime->get_commit(session_id, &commit)) {
        printf("commit: %s\n", commit.text);
        rime->free_commit(&commit);
    }

    if (rime->get_status(session_id, &status)) {
        print_status(&status);
        rime->free_status(&status);
    }

    if (rime->get_context(session_id, &context)) {
        print_context(&context);
        rime->free_context(&context);
    }
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input>\n", argv[0]);
        return 1;
    }
    std::string input = argv[1];
    RimeApi* rime = rime_get_api();

    RIME_STRUCT(RimeTraits, traits);
    traits.app_name = "rime.console";
    rime->setup(&traits);

    rime->set_notification_handler(&on_message, NULL);

    fprintf(stderr, "initializing...\n");
[[maybe_unused]] reload:

    rime->initialize(NULL);
    Bool full_check = True;
    if (rime->start_maintenance(full_check))
        rime->join_maintenance_thread();
    fprintf(stderr, "ready.\n");

    RimeSessionId session_id = 0;
    if (!rime->find_session(session_id) &&
        !(session_id = ensure_session(rime))) {
        fprintf(stderr, "can not get session.\n");
        return 1;
    }

    RimeApi* rime2 = rime_get_api();
    if (rime2->simulate_key_sequence(session_id, input.c_str())) {
        print(session_id);
    }

}