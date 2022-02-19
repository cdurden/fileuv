#include <uv.h>
#include <unistd.h>
#include <Rcpp.h>
#include <later_api.h>
#include "fileuv.h"
//https://cran.r-project.org/web/packages/later/vignettes/later-cpp.html
//https://mfasiolo.github.io/sc2-2019/rcpp/3_rcpp/

Rcpp::RawVector to_raw(const char* str) {
    Rcpp::RawVector res(str, str + std::strlen(str));
    return res;
}

void notify(listener_data_t *listener_data_ptr) {
      Rcpp::Environment base("package:fileuv");
      Rcpp::Function trigger_r = base["trigger"];
      trigger_r(listener_data_ptr->path);
}

void run_callback(void *listener_data_ptr) {
    (*((listener_data_t *) listener_data_ptr)->callback)();
}

void on_fs_event(uv_fs_event_t *handle, const char *filename, int events, int status) {
    uv_fs_event_stop(handle);
    return;
}

class Listen : public later::BackgroundTask {
  public:
    Listen(void *_listener_data_ptr) :
      listener_data_ptr((listener_data_t *) _listener_data_ptr)
    { }

  protected:
    void execute() {
      uv_loop_t *loop = (uv_loop_t *) malloc(sizeof(uv_loop_t));
      uv_loop_init(loop);

      uv_fs_event_t *fs_event_req = (uv_fs_event_t *) malloc(sizeof(uv_fs_event_t));
      uv_fs_event_init(loop, fs_event_req);
      // The recursive flag watches subdirectories too.
      uv_fs_event_start(fs_event_req, on_fs_event, listener_data_ptr->path, UV_FS_EVENT_RECURSIVE);
      uv_run(loop, UV_RUN_ONCE);
      free(fs_event_req);
    }

    void complete() {
      (new Listen((void *) listener_data_ptr))->begin();
      notify(listener_data_ptr);
    }

  private:
    listener_data_t *listener_data_ptr;
};


// [[Rcpp::export]]
listener_data_t *listen(const char* path) {
  listener_data_t *listener_data_ptr;
  listener_data_ptr = (listener_data_t*) calloc(1, sizeof(listener_data_t));
  listener_data_ptr->path = path;
  (new Listen((void *) listener_data_ptr))->begin();
  return listener_data_ptr;
}
