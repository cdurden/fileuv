handlers <- new.env()
addFileEventHandler <- function(path, handler) {
    handlers[[path]] <- append(handlers[[path]], handler)
}
trigger <- function(path) {
    lapply(handlers[[path]], function(handler) handler())
}
