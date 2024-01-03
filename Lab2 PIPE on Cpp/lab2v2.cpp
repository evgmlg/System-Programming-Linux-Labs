#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

#define BUFFER_SIZE 1024

int main() {
    int fd[2];
    if (pipe(fd) == -1) {
        std::cerr << "Error creating pipe\n";
        return 1;
    }

    for (int i = 0; i < 4; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Error creating child process\n";
            return 1;
        } else if (pid == 0) {
            // Child process
            if (i == 0) {
                // Consumer process
                close(fd[1]); // Close write end of pipe
                char buffer[BUFFER_SIZE];
                while (true) {
                    int bytes_read = read(fd[0], buffer, BUFFER_SIZE);
                    if (bytes_read == -1) {
                        std::cerr << "Error reading from pipe\n";
                        return 1;
                    } else if (bytes_read == 0) {
                        break; // End of file
                    } else {
                        std::cout << "Consumer: " << buffer;
                    }
                }
                close(fd[0]); // Close read end of pipe
                return 0;
            } else {
                // Producer process
                close(fd[0]); // Close read end of pipe
                const char* message = "Hello, world!\n";
                if (write(fd[1], message, strlen(message)) == -1) {
                        std::cerr << "Error writing to pipe\n";
                        return 1;
                    }
                close(fd[1]); // Close write end of pipe
                return 0;
            }
        }
    }

    // Parent process
    close(fd[0]); // Close read end of pipe
    close(fd[1]); // Close write end of pipe
    for (int i = 0; i < 5; ++i) {
        int status;
        waitpid(-1, &status, 0); // Wait for child process to finish
    }
    return 0;
}
