#pragma once

#include "context.h"

namespace blobby
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