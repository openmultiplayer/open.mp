#include "objects_impl.hpp"

void ObjectComponent::onTick(Microseconds elapsed, TimePoint now)
{
    for (auto it = processedObjects.begin(); it != processedObjects.end();) {
        Object* obj = *(it++);
        if (obj->advance(elapsed, now)) {
            ScopedPoolReleaseLock lock(*this, *obj);
            eventDispatcher.dispatch(&ObjectEventHandler::onMoved, *obj);
        }
    }

    for (auto it = processedPlayerObjects.begin(); it != processedPlayerObjects.end();) {
        PlayerObject* obj = *(it++);
        if (obj->advance(elapsed, now)) {
            ScopedPoolReleaseLock lock(obj->getObjects(), *obj);
            eventDispatcher.dispatch(&ObjectEventHandler::onPlayerObjectMoved, obj->getObjects().getPlayer(), *obj);
        }
    }
}

void ObjectComponent::onConnect(IPlayer& player)
{
    player.addData(new PlayerObjectData(*this, player));

    for (IObject* o : storage) {
        Object* obj = static_cast<Object*>(o);
        obj->createForPlayer(player);
    }
}

COMPONENT_ENTRY_POINT()
{
    return new ObjectComponent();
}
