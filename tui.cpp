#include "tui.hpp"
#include <ncurses.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

Launcher::Launcher() {
    AppSearcher s;
    allApps = s.loadApps();
    filtered = allApps;
}

void Launcher::draw() {
    clear();
    
    // Interface mais limpa, sem mencionar ESC
    
    mvprintw(1, 1, ">  ");

        if (query.empty()) {
        attron(A_DIM); // deixa o texto mais claro (placeholder visual)
        mvprintw(1, 3, "Search...");
        attroff(A_DIM);
    } else {
        mvprintw(1, 3, "%s", query.c_str());
    }

    if (!query.empty()) {
      

      for (int i = 0; i < (int)filtered.size(); i++) {
        if (i == selected) attron(A_REVERSE);
        mvprintw(i + 3, 2, "%s", filtered[i].name.c_str());
        if (i == selected) attroff(A_REVERSE);
      }
    }

    refresh();
}

void Launcher::update_list(const std::vector<AppInfo>& newList) {
  filtered.clear();
  for (int i = 0; i < (int)newList.size() && i < 4; i++) {
    filtered.push_back(newList[i]);
  }
    selected = 0;
}

bool Launcher::launchApp(const AppInfo& app) {
    endwin(); // Fecha ncurses completamente
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Processo filho - vai executar o aplicativo
        setsid(); // Cria nova sessão para desvincular do terminal
        
        // Fecha todos os file descriptors herdados
        for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
            close(fd);
        }
        
        // Redireciona stdin/stdout/stderr para /dev/null
        int nullfd = open("/dev/null", O_RDWR);
        if (nullfd != -1) {
            dup2(nullfd, STDIN_FILENO);
            dup2(nullfd, STDOUT_FILENO);
            dup2(nullfd, STDERR_FILENO);
            close(nullfd);
        }
        
        // Remove variáveis de ambiente que podem interferir
        unsetenv("TERM");
        unsetenv("TERMINFO");
        unsetenv("TERMCAP");
        
        // Executa o aplicativo
        // Usa shell para suportar argumentos e variáveis
        execl("/bin/sh", "sh", "-c", app.exec.c_str(), (char*)NULL);
        
        // Se falhar
        _exit(1);
    } else if (pid > 0) {
        // Processo pai (launcher) - apenas sai
        return true;
    }
    
    return false;
}

bool Launcher::handleKey(int ch) {
    if (ch == 10) { // Enter
        if (!filtered.empty() && selected >= 0 && selected < (int)filtered.size()) {
            return launchApp(filtered[selected]);
        }
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
        if (!query.empty())
            query.pop_back();
        AppSearcher s;
        update_list(s.filter(allApps, query));
    } else if (ch >= 32 && ch <= 126) { // Caracteres imprimíveis
        query.push_back((char)ch);
        AppSearcher s;
        update_list(s.filter(allApps, query));
    } else if (ch == KEY_UP) {
        if (selected > 0) selected--;
    } else if (ch == KEY_DOWN) {
        if (selected < (int)filtered.size() - 1) selected++;
    } else if (ch == 27) { // ESC - fecha sem fazer nada
        return true;
    } else if (ch == 12 || ch == KEY_CLEAR) { // Ctrl+L
        query.clear();
        filtered = allApps;
        selected = 0;
    }
    
    return false;
}

void Launcher::run() {
    initscr();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    bool shouldExit = false;
    
    while (!shouldExit) {
        draw();
        
        int ch = getch();
        if (ch == ERR) continue;
        
        shouldExit = handleKey(ch);
    }
    
    endwin();
}
