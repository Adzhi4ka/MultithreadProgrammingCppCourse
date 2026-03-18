#pragma once

namespace domain::notifications::services {

    class NotificationService {

            
        public:

            template <typename AppEventBus>
            NotificationService(AppEventBus& eventBus);

    };

}