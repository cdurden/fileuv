#include <uv.h>
#include <unistd.h>
#include <Rcpp.h>
#include <later_api.h>
#include "fileuv.h"
//https://cran.r-project.org/web/packages/later/vignettes/later-cpp.html
//https://mfasiolo.github.io/sc2-2019/rcpp/3_rcpp/

/*
typedef struct listener_data {
    Rcpp::Function *callback;
    const char *path;
    SEXP self;
} listener_data_t;
*/
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
    //fprintf(stderr, "Event on file: %s\n", filename);
    uv_fs_event_stop(handle);
    return;
}

void print_callback_info(listener_data_t *listener_data_ptr) {
      //fprintf(stderr, "Listener data address: %p\n", listener_data_ptr);
      /*
      Rcpp::Environment base("package:base");
      Rcpp::Function typeof_r = base["typeof"];
      Rcpp::Function ls_r = base["ls"];
      Rcpp::Function cat_r = base["cat"];
      Rcpp::CharacterVector res = typeof_r(*listener_data_ptr->callback);
      fprintf(stderr, "Callback address: %p\n", listener_data_ptr->callback);
      std::cout << "typeof(callback) = " << Rcpp::as<std::string>(res);
      if (Rcpp::as<std::string>(res) == "environment") {

          Rcpp::CharacterVector items = ls_r(*listener_data_ptr->callback);
          cat_r(items);
      }
      */
}


class Listen : public later::BackgroundTask {
  public:
    Listen(void *_listener_data_ptr) :
      listener_data_ptr((listener_data_t *) _listener_data_ptr)
    { }

  protected:
    void execute() {
      //uv_loop_t *loop = uv_default_loop();
      uv_loop_t *loop = (uv_loop_t *) malloc(sizeof(uv_loop_t));
      uv_loop_init(loop);

      uv_fs_event_t *fs_event_req = (uv_fs_event_t *) malloc(sizeof(uv_fs_event_t));
      uv_fs_event_init(loop, fs_event_req);
      // The recursive flag watches subdirectories too.
      uv_fs_event_start(fs_event_req, on_fs_event, listener_data_ptr->path, UV_FS_EVENT_RECURSIVE);
      uv_run(loop, UV_RUN_ONCE);
      //fprintf(stderr, "Listener loop finished");
      free(fs_event_req);
    }

    void complete() {
      //fprintf(stderr, "Restarting listener\n");
      (new Listen((void *) listener_data_ptr))->begin();
      //print_callback_info(listener_data_ptr);
      notify(listener_data_ptr);
      //fprintf(stderr, "typeof(callback) = %s\n",
      //print_callback_info(listener_data_ptr);
      //(*((listener_data_t *) listener_data_ptr)->callback)();
      //later::later(run_callback, (void *) listener_data_ptr, 0);
    }

  private:
    listener_data_t *listener_data_ptr;
};


// [[Rcpp::export]]
//listener_data_t *listen(const char* path, Rcpp::Function callback) {
listener_data_t *listen(const char* path) {
  //PROTECT(callback);
  listener_data_t *listener_data_ptr;
  //R_PreserveObject(callback);
  listener_data_ptr = (listener_data_t*) calloc(1, sizeof(listener_data_t));
  //listener_data_ptr->callback = &callback;
  listener_data_ptr->path = path;
  /*
  fprintf(stderr, "Callback address: %p\n", &callback);
  fprintf(stderr, "Path address: %p\n", path);
  fprintf(stderr, "Path: %s\n", path);
  */
  //print_callback_info(listener_data_ptr);
  // See https://homepage.divms.uiowa.edu/~luke/R/simpleref.html#toc5 for information about R_MakeExternalPtr
  // This is used in the following projects
  // httpuv/src/webapplication.cpp:357
  // background/src/async.c:125
  //R_PreserveObject(listener_data_ptr->self = R_MakeExternalPtr(listener_data_ptr, R_NilValue, R_NilValue));
  (new Listen((void *) listener_data_ptr))->begin();
  return listener_data_ptr;
  //return listener_data_ptr->self;
}
