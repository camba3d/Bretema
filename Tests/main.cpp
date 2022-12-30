#include "Bretema/btm_app.hpp"

int main()
{
    btm::App bretema { "Bretema" };

    while (true)  // Allows app restart
    {
        bretema.run();
        bretema.cleanup();

        if (bretema.isMarkedToClose())  // Ensures app close
            break;

        bretema.reset();
    }

    return 0;
}