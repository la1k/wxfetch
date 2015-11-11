add_executable(wxdecode main.c dsp.c filter.c)
target_link_libraries(wxdecode m sndfile)
