typedef struct listener_data {
    Rcpp::Function *callback;
    const char *path;
    SEXP self;
} listener_data_t;
