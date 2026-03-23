#include <filesystem>
#include <fstream>
#include <vector>
#include <atomic>
#include <thread>
#include <cstdint>
#include <optional>
#include <condition_variable>

//#include "pinyin.h"
#include "ime.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

// for better implementation, check
// https://github.com/libpinyin/ibus-libpinyin.git

#ifdef USE_LIBRIME
#include <rime_api.h>

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

struct _IME_Instance final
{
    std::string input;
    std::string prefix;

    std::optional<size_t> selection;
    std::vector<std::string> candidates;
    std::vector<std::pair<std::string, int>> stk; // (sentence, start)

    bool done;
    std::thread th;

    mutable std::mutex mtx;
    mutable std::condition_variable cond;

//    pinyin_context_t *context;
//    pinyin_instance_t *instance;

    RimeTraits traits{0};

    _IME_Instance()
    {
        RimeApi* rime = rime_get_api();

        RIME_STRUCT_INIT(RimeTraits, traits);
        traits.app_name = "rime.console";
        rime->setup(&traits);
        rime->set_notification_handler(&on_message, NULL);
        rime->initialize(NULL);
        Bool full_check = True;
        if (rime->start_maintenance(full_check))
            rime->join_maintenance_thread();

        RimeSessionId session_id = 0;
        if (!rime->find_session(session_id) &&
            !(session_id = ensure_session(rime))) {
            fprintf(stderr, "can not get session.\n");
        }

        fflassert(session_id);

        done = false;
        th = std::thread([this, session_id]()
        {
            while(!done){
                std::unique_lock<std::mutex> lock(mtx);
                cond.wait(lock);

                RimeApi* rime = rime_get_api();
                if(done || rime->is_maintenance_mode()){
                    return;
                }

                if(input.empty()){
                    prefix.clear();

                    selection .reset();
                    candidates.clear();
                    stk       .clear();
                    continue;
                }

                if(selection.has_value()) {
                    //     uint num = 0;
                    //     pinyin_get_n_candidate(instance, &num);
                    //
                    //     if (selection.value() >= num)
                    //     {
                    //         selection.reset();
                    //         continue;
                    //     }
                    //
                    //     const auto choice = selection.value();
                    //     selection.reset();
                    //
                    //     lookup_candidate_t *candidate = nullptr;
                    //     pinyin_get_candidate(instance, choice, &candidate);
                    //
                    //     const char *word = nullptr;
                    //     pinyin_get_candidate_string(instance, candidate, &word);
                    //
                    //     lookup_candidate_type_t type;
                    //     pinyin_get_candidate_type(instance, candidate, &type);
                    //
                    //      const auto [sentence, offset] = [type, word, this]() -> std::pair<std::string, size_t>
                    //      {
                    //          if((type == NBEST_MATCH_CANDIDATE) || stk.empty()){
                    //              return {word, 0};
                    //          }
                    //          else{
                    //              return {stk.back().first + word, to_uz(stk.back().second)};
                    //          }
                    //      }();
                    //
                    //      stk.emplace_back(sentence, pinyin_choose_candidate(instance, offset, candidate));
                    // }
                } else if(stk.empty()){
                    //pinyin_parse_more_full_pinyins(instance, input.c_str());
                    //pinyin_guess_sentence_with_prefix(instance, prefix.c_str());

                    if (rime->simulate_key_sequence(session_id, input.c_str())) {
                        RIME_STRUCT(RimeCommit, commit);
                        RIME_STRUCT(RimeStatus, status);
                        RIME_STRUCT(RimeContext, context);

                        if (rime->get_commit(session_id, &commit)) {
                            printf("commit: %s\n", commit.text);
                            rime->free_commit(&commit);
                        }

                        if (rime->get_status(session_id, &status)) {
                            //print_status(&status);
                            rime->free_status(&status);
                        }

                        if (rime->get_context(session_id, &context)) {
                            if (context.composition.length > 0 || context.menu.num_candidates > 0) {
                                // const char* preedit = context.composition.preedit;
                                // if (!preedit)
                                //     return;
                                // size_t len = strlen(preedit);
                                // size_t start = context.composition.sel_start;
                                // size_t end = context.composition.sel_end;
                                // size_t cursor = context.composition.cursor_pos;
                                // for (size_t i = 0; i <= len; ++i) {
                                //     if (start < end) {
                                //         if (i == start) {
                                //             putchar('[');
                                //         } else if (i == end) {
                                //             putchar(']');
                                //         }
                                //     }
                                //     if (i == cursor)
                                //         putchar('|');
                                //     if (i < len)
                                //         candidates.emplace_back(preedit);
                                // }

                                if (context.menu.num_candidates == 0)
                                    return;
                                printf("page: %d%c (of size %d)\n", context.menu.page_no + 1,
                                       context.menu.is_last_page ? '$' : ' ', context.menu.page_size);
                                for (int i = 0; i < context.menu.num_candidates; ++i) {
                                    bool highlighted = i == context.menu.highlighted_candidate_index;
                                    printf("%d. %c%s%c%s\n", i + 1, highlighted ? '[' : ' ',
                                           context.menu.candidates[i].text, highlighted ? ']' : ' ',
                                           context.menu.candidates[i].comment ? context.menu.candidates[i].comment : "");
                                    candidates.emplace_back(context.menu.candidates[i].text);
                                }
                            }
                            rime->free_context(&context);
                        }
                    }
                }
                //
                // candidates.clear();
                // pinyin_guess_candidates(instance, stk.empty() ? 0 : stk.back().second, SORT_BY_PHRASE_LENGTH_AND_PINYIN_LENGTH_AND_FREQUENCY);
                //
                // guint num = 0;
                // pinyin_get_n_candidate(instance, &num);
                //
                // for(guint i = 0; i < num; ++i){
                //     lookup_candidate_t *candidate = nullptr;
                //     pinyin_get_candidate(instance, i, &candidate);
                //
                //     const char *word = nullptr;
                //     pinyin_get_candidate_string(instance, candidate, &word);
                //
                //     candidates.emplace_back(word);
                // }
            }
        });
    }

    ~_IME_Instance()
    {
        {
            const std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cond.notify_one();

        th.join();

    }

    // return true if input pinyin string is complete, i.e.
    //
    //   complete: nihaoya
    // incomplete: nihaoy, nhya
    //
    // we skip the training and saving if input is incomplete
    bool is_input_complete_pinyin()
    {
        return true;
    }
};
#else
struct _IME_Instance final
{
    std::string input;
    std::string prefix;

    std::optional<size_t> selection;
    std::vector<std::string> candidates;
    std::vector<std::pair<std::string, int>> stk; // (sentence, start)

    bool done;
    std::thread th;

    mutable std::mutex mtx;
    mutable std::condition_variable cond;

    pinyin_context_t *context;
    pinyin_instance_t *instance;

    _IME_Instance()
    {
        // suppress warning message: open libpinyin/conf/user.conf failed.
        // create libpinyin/data/user.conf if not exists

        if(const std::filesystem::path conf = "libpinyin/conf/user.conf"; !std::filesystem::exists(conf)){
            std::filesystem::create_directories(conf.parent_path());
            std::ofstream{conf};
        }

        context = pinyin_init("libpinyin/data", "libpinyin/conf");
        fflassert(context);

        pinyin_set_options(context,
                static_cast<guint32>(PINYIN_INCOMPLETE ) |
                 static_cast<guint32>(PINYIN_CORRECT_ALL) |
                 static_cast<guint32>(USE_DIVIDED_TABLE ) |
                 static_cast<guint32>(USE_RESPLIT_TABLE ) |
                 static_cast<guint32>(DYNAMIC_ADJUST    ));

         instance = pinyin_alloc_instance(context);
         fflassert(instance);

        done = false;
        th = std::thread([this]()
        {
            while(!done){
                std::unique_lock<std::mutex> lock(mtx);
                cond.wait(lock);

                if(done){
                    return;
                }

                if(input.empty()){
                    prefix.clear();

                    selection .reset();
                    candidates.clear();
                    stk       .clear();

                    pinyin_reset(instance);
                    continue;
                }

                if(selection.has_value()){
                    uint num = 0;
                    pinyin_get_n_candidate(instance, &num);

                    if (selection.value() >= num)
                    {
                        selection.reset();
                        continue;
                    }

                    const auto choice = selection.value();
                    selection.reset();

                    lookup_candidate_t *candidate = nullptr;
                    pinyin_get_candidate(instance, choice, &candidate);

                    const char *word = nullptr;
                    pinyin_get_candidate_string(instance, candidate, &word);

                    lookup_candidate_type_t type;
                    pinyin_get_candidate_type(instance, candidate, &type);

                     const auto [sentence, offset] = [type, word, this]() -> std::pair<std::string, size_t>
                     {
                         if((type == NBEST_MATCH_CANDIDATE) || stk.empty()){
                             return {word, 0};
                         }
                         else{
                             return {stk.back().first + word, to_uz(stk.back().second)};
                         }
                     }();

                     stk.emplace_back(sentence, pinyin_choose_candidate(instance, offset, candidate));
                }
                else if(stk.empty()){
                    pinyin_parse_more_full_pinyins(instance, input.c_str());
                    pinyin_guess_sentence_with_prefix(instance, prefix.c_str());
                }

                candidates.clear();
                pinyin_guess_candidates(instance, stk.empty() ? 0 : stk.back().second, SORT_BY_PHRASE_LENGTH_AND_PINYIN_LENGTH_AND_FREQUENCY);

                guint num = 0;
                pinyin_get_n_candidate(instance, &num);

                for(guint i = 0; i < num; ++i){
                    lookup_candidate_t *candidate = nullptr;
                    pinyin_get_candidate(instance, i, &candidate);

                    const char *word = nullptr;
                    pinyin_get_candidate_string(instance, candidate, &word);

                    candidates.emplace_back(word);
                }
            }
        });
    }

    ~_IME_Instance()
    {
        {
            const std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cond.notify_one();

        th.join();

        pinyin_free_instance(instance);
        pinyin_mask_out(context, 0x0, 0x0);

        pinyin_save(context);
        pinyin_fini(context);
    }

    // return true if input pinyin string is complete, i.e.
    //
    //   complete: nihaoya
    // incomplete: nihaoy, nhya
    //
    // we skip the training and saving if input is incomplete
    bool is_input_complete_pinyin()
    {
        size_t i = 0;
         size_t length = pinyin_get_parsed_input_length(instance);

         for(; i < length; ++i){
             ChewingKey *key = nullptr;
             if(pinyin_get_pinyin_key(instance, i, &key) && key){
                 if(pinyin_get_pinyin_is_incomplete(instance, key)){
                     return false;
                 }
             }
         }
        return true;
    }
};
#endif

static _IME_Instance * asIMEPtr(void *ptr)
{
    return reinterpret_cast<_IME_Instance *>(ptr);
}

IME::IME()
    : m_instance(new _IME_Instance())
{}

IME::~IME()
{
    delete asIMEPtr(m_instance);
}

void IME::clear()
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->input.clear();
    }
    imePtr->cond.notify_one();
}

void IME::feed(char ch)
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->input.push_back(ch);
    }
    imePtr->cond.notify_one();
}

void IME::backspace()
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        if(imePtr->input.empty()){
            return;
        }

        if(imePtr->stk.empty()){
            imePtr->input.pop_back();
        }
        else{
            imePtr->stk.pop_back();
        }
    }
    imePtr->cond.notify_one();
}

void IME::assign(std::string argPrefix, std::string argInput)
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->stk.clear();
        imePtr->selection.reset();

        imePtr->prefix = std::move(argPrefix);
        imePtr->input  = std::move(argInput);
    }
    imePtr->cond.notify_one();
}

void IME::select(size_t index)
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->selection = index;
    }
    imePtr->cond.notify_one();
}

bool IME::done() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return !imePtr->stk.empty() && to_uz(imePtr->stk.back().second) >= imePtr->input.size();
}

bool IME::empty() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return imePtr->input.empty();
}

std::string IME::input() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return imePtr->input;
}

std::string IME::result() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    if(imePtr->stk.empty()){
        return imePtr->input;
    }
    return imePtr->stk.back().first + imePtr->input.substr(imePtr->stk.back().second);
}

std::string IME::sentence() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    if(imePtr->stk.empty()){
        return {};
    }
    return imePtr->stk.back().first;
}

std::vector<std::string> IME::candidates() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return imePtr->candidates;
}
