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
        Context m_context;
    };
}