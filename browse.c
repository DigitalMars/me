#if _WIN32

#include	<windows.h>
#include	<winuser.h>

#pragma includelib "shell32.lib"

void browse(const char *s)
{
    ShellExecuteA(NULL, "open", s, NULL, NULL, SW_SHOWNORMAL);
}

#elif 0
else version (OSX)
{
    import core.stdc.stdio;
    import core.stdc.string;
    import core.sys.posix.unistd;

    void browse(string url)
    {
        const(char)*[5] args;

        auto curl = url.tempCString();
        const(char)* browser = core.stdc.stdlib.getenv("BROWSER");
        if (browser)
        {   browser = strdup(browser);
            args[0] = browser;
            args[1] = curl;
            args[2] = null;
        }
        else
        {
            args[0] = "open".ptr;
            args[1] = curl;
            args[2] = null;
        }

        auto childpid = fork();
        if (childpid == 0)
        {
            core.sys.posix.unistd.execvp(args[0], cast(char**)args.ptr);
            perror(args[0]);                // failed to execute
            return;
        }
        if (browser)
            free(cast(void*)browser);
    }
}
else version (Posix)
{
    import core.stdc.stdio;
    import core.stdc.string;
    import core.sys.posix.unistd;

    void browse(string url)
    {
        const(char)*[3] args;

        const(char)* browser = core.stdc.stdlib.getenv("BROWSER");
        if (browser)
        {   browser = strdup(browser);
            args[0] = browser;
        }
        else
            //args[0] = "x-www-browser".ptr;  // doesn't work on some systems
            args[0] = "xdg-open".ptr;

        args[1] = url.tempCString();
        args[2] = null;

        auto childpid = fork();
        if (childpid == 0)
        {
            core.sys.posix.unistd.execvp(args[0], cast(char**)args.ptr);
            perror(args[0]);                // failed to execute
            return;
        }
        if (browser)
            free(cast(void*)browser);
    }
}
#else
void browse(const char *s)
{
}
#endif
