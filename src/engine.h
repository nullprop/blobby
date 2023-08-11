#pragma once

#include "context.h"

namespace organic
{
    class Engine
    {
      public:
        Engine();
        ~Engine();

        void Loop();

      private:
        void Resize(int width, int height);

        Context m_context;
    };
}