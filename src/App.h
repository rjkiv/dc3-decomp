#pragma once

class App {
private:
    int _pad;

protected:
    void DrawRegular();
    void CaptureHiRes();

public:
    App(int argc, char **argv);
    ~App(void);

    void RunWithoutDebugging();
    void Run();
};
