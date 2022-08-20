/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

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

void ObjectComponent::onPlayerConnect(IPlayer& player)
{
    player.addExtension(new PlayerObjectData(*this, player), true);

    static bool artwork = *core->getConfig().getBool("artwork.enabled");

    if (artwork && player.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL)
        return;

    for (IObject* o : storage) {
        Object* obj = static_cast<Object*>(o);
        obj->createForPlayer(player);
    }
}

void ObjectComponent::onPlayerFinishedDownloading(IPlayer& player)
{
    // Restream objects with custom models.
    for (IObject* o : storage) {
        Object* obj = static_cast<Object*>(o);
        obj->createForPlayer(player);
    }
}

COMPONENT_ENTRY_POINT()
{
    return new ObjectComponent();
}
