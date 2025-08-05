/*
 * This file is part of JCWM.
 *
 * JCWM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * JCWM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with JCWM. If not, see <https://www.gnu.org/licenses/>.
 */

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