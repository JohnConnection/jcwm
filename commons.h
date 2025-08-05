#ifndef COMMONS_H
#define COMMONS_H
    #define PANEL_HEIGHT 24
    //change this to your thing
#define LAUNCHER "gmrun"

void launch(const char* name)
{
    pid_t pid = fork();
    if(pid == 0)
    {
        setsid();
        execlp(name, name,NULL);
        perror("execlp fail");
        _exit(-1);
    } else if(pid < 0)
    {
        perror("fork fail");
    }
}
#endif