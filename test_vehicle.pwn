#include <a_samp>

public OnGameModeInit()
{
    const new BaseVehicle = 411;
    const new CustomVehicle = 40001;

    // Test AddVehicleModel
    // Base vehicle ID 411 (Infernus), custom ID 40001
    if (AddVehicleModel(BaseVehicle, CustomVehicle, "infernus.dff", "infernus.txd"))
    {
        print("Custom vehicle model registered successfully!");
    }
    else
    {
        print("AddVehicleModel failed or model already exists. Using existing registration.");
    }

    new vehicle = CreateVehicle(CustomVehicle, 0.0, 0.0, 5.0, 0.0, 0, 0, 30);
    if (vehicle == INVALID_VEHICLE_ID)
    {
        print("CreateVehicle failed for custom model %d.", CustomVehicle);
    }
    else
    {
        print("Created custom vehicle %d with id %d.", CustomVehicle, vehicle);
    }

    return 1;
}

public OnPlayerRequestDownload(playerid, type, checksum)
{
    print("OnPlayerRequestDownload: player=%d type=%d checksum=%d", playerid, type, checksum);
    return 1;
}

public OnPlayerFinishedDownloading(playerid)
{
    print("OnPlayerFinishedDownloading: player=%d", playerid);
    return 1;
}