#pragma once

#include "vehicles.hpp"
#include <types.hpp>

namespace Impl {

inline bool isValidComponentForVehicleModel(int vehicleModel, int componentId)
{
    static const StaticArray<uint8_t, MAX_VEHICLE_COMPONENTS> componentValidVehicleModel = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xA0, 0xA0, 0xA0, 0xA0,
        0xA0, 0xA0, 0xA0, 0xA0, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2,
        0xA2, 0xA2, 0xAF, 0xAF, 0xAF, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
        0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1,
        0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F,
        0x9F, 0x9F, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9E, 0x9E,
        0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0xFF, 0xFF, 0xFF, 0xAF,
        0x86, 0x86, 0xA7, 0x88, 0x88, 0x88, 0x86, 0x88, 0x88, 0x87,
        0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87,
        0x87, 0x87, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x88, 0xA7,
        0xA7, 0xA7, 0xA7, 0xA7, 0xB0, 0xB0, 0xB0, 0xB0, 0xA0, 0xA0,
        0xA0, 0xA0, 0xFF, 0xFF, 0xFF, 0xFF, 0xA2, 0xA2, 0xA2, 0xA2,
        0xA5, 0xA5, 0xA5, 0xA5, 0xA1, 0xA1, 0xA1, 0xA1, 0x9F, 0x9F,
        0x9F, 0x9F, 0x9F, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0xA0,
        0xA0, 0xA2, 0xA2, 0x9F, 0xAF, 0xAF, 0xAF, 0xAF, 0x86, 0x86,
        0x86, 0x88, 0x88, 0x88, 0x88, 0x86, 0xA7, 0xA7, 0xA7, 0xA7,
        0xB0, 0xB0, 0xB0, 0xB0
    };

    componentId -= 1000;
    if (0 > componentId || componentId > MAX_VEHICLE_COMPONENTS) {
        return false;
    }
    auto componentModel = componentValidVehicleModel[componentId];
    return componentModel == 0xFF || componentModel == vehicleModel - 400;
}

inline int getVehicleComponentSlot(int component)
{
    static const StaticArray<VehicleComponentSlot, MAX_VEHICLE_COMPONENTS> allComponentSlots = {
        VehicleComponent_Spoiler, // 1000
        VehicleComponent_Spoiler, // 1001
        VehicleComponent_Spoiler, // 1002
        VehicleComponent_Spoiler, // 1003
        VehicleComponent_Hood, // 1004
        VehicleComponent_Hood, // 1005
        VehicleComponent_Roof, // 1006
        VehicleComponent_SideSkirt, // 1007
        VehicleComponent_Nitro, // 1008
        VehicleComponent_Nitro, // 1009
        VehicleComponent_Nitro, // 1010
        VehicleComponent_Hood, // 1011
        VehicleComponent_Hood, // 1012
        VehicleComponent_Lamps, // 1013
        VehicleComponent_Spoiler, // 1014
        VehicleComponent_Spoiler, // 1015
        VehicleComponent_Spoiler, // 1016
        VehicleComponent_SideSkirt, // 1017
        VehicleComponent_Exhaust, // 1018
        VehicleComponent_Exhaust, // 1019
        VehicleComponent_Exhaust, // 1020
        VehicleComponent_Exhaust, // 1021
        VehicleComponent_Exhaust, // 1022
        VehicleComponent_Spoiler, // 1023
        VehicleComponent_Lamps, // 1024
        VehicleComponent_Wheels, // 1025
        VehicleComponent_SideSkirt, // 1026
        VehicleComponent_SideSkirt, // 1027
        VehicleComponent_Exhaust, // 1028
        VehicleComponent_Exhaust, // 1029
        VehicleComponent_SideSkirt, // 1030
        VehicleComponent_SideSkirt, // 1031
        VehicleComponent_Roof, // 1032
        VehicleComponent_Roof, // 1033
        VehicleComponent_Exhaust, // 1034
        VehicleComponent_Roof, // 1035
        VehicleComponent_SideSkirt, // 1036
        VehicleComponent_Exhaust, // 1037
        VehicleComponent_Roof, // 1038
        VehicleComponent_SideSkirt, // 1039
        VehicleComponent_SideSkirt, // 1040
        VehicleComponent_SideSkirt, // 1041
        VehicleComponent_SideSkirt, // 1042
        VehicleComponent_Exhaust, // 1043
        VehicleComponent_Exhaust, // 1044
        VehicleComponent_Exhaust, // 1045
        VehicleComponent_Exhaust, // 1046
        VehicleComponent_SideSkirt, // 1047
        VehicleComponent_SideSkirt, // 1048
        VehicleComponent_Spoiler, // 1049
        VehicleComponent_Spoiler, // 1050
        VehicleComponent_SideSkirt, // 1051
        VehicleComponent_SideSkirt, // 1052
        VehicleComponent_Roof, // 1053
        VehicleComponent_Roof, // 1054
        VehicleComponent_Roof, // 1055
        VehicleComponent_SideSkirt, // 1056
        VehicleComponent_SideSkirt, // 1057
        VehicleComponent_Spoiler, // 1058
        VehicleComponent_Exhaust, // 1059
        VehicleComponent_Spoiler, // 1060
        VehicleComponent_Roof, // 1061
        VehicleComponent_SideSkirt, // 1062
        VehicleComponent_SideSkirt, // 1063
        VehicleComponent_Exhaust, // 1064
        VehicleComponent_Exhaust, // 1065
        VehicleComponent_Exhaust, // 1066
        VehicleComponent_Roof, // 1067
        VehicleComponent_Roof, // 1068
        VehicleComponent_SideSkirt, // 1069
        VehicleComponent_SideSkirt, // 1070
        VehicleComponent_SideSkirt, // 1071
        VehicleComponent_SideSkirt, // 1072
        VehicleComponent_Wheels, // 1073
        VehicleComponent_Wheels, // 1074
        VehicleComponent_Wheels, // 1075
        VehicleComponent_Wheels, // 1076
        VehicleComponent_Wheels, // 1077
        VehicleComponent_Wheels, // 1078
        VehicleComponent_Wheels, // 1079
        VehicleComponent_Wheels, // 1080
        VehicleComponent_Wheels, // 1081
        VehicleComponent_Wheels, // 1082
        VehicleComponent_Wheels, // 1083
        VehicleComponent_Wheels, // 1084
        VehicleComponent_Wheels, // 1085
        VehicleComponent_Stereo, // 1086
        VehicleComponent_Hydraulics, // 1087
        VehicleComponent_Roof, // 1088
        VehicleComponent_Exhaust, // 1089
        VehicleComponent_SideSkirt, // 1090
        VehicleComponent_Roof, // 1091
        VehicleComponent_Exhaust, // 1092
        VehicleComponent_SideSkirt, // 1093
        VehicleComponent_SideSkirt, // 1094
        VehicleComponent_SideSkirt, // 1095
        VehicleComponent_Wheels, // 1096
        VehicleComponent_Wheels, // 1097
        VehicleComponent_Wheels, // 1098
        VehicleComponent_SideSkirt, // 1099
        VehicleComponent_FrontBullbar, // 1100
        VehicleComponent_SideSkirt, // 1101
        VehicleComponent_SideSkirt, // 1102
        VehicleComponent_Roof, // 1103
        VehicleComponent_Exhaust, // 1104
        VehicleComponent_Exhaust, // 1105
        VehicleComponent_SideSkirt, // 1106
        VehicleComponent_SideSkirt, // 1107
        VehicleComponent_SideSkirt, // 1108
        VehicleComponent_RearBullbar, // 1109
        VehicleComponent_RearBullbar, // 1110
        VehicleComponent_FrontBumper, // 1111
        VehicleComponent_FrontBumper, // 1112
        VehicleComponent_Exhaust, // 1113
        VehicleComponent_Exhaust, // 1114
        VehicleComponent_FrontBullbar, // 1115
        VehicleComponent_FrontBullbar, // 1116
        VehicleComponent_FrontBumper, // 1117
        VehicleComponent_SideSkirt, // 1118
        VehicleComponent_SideSkirt, // 1119
        VehicleComponent_SideSkirt, // 1120
        VehicleComponent_SideSkirt, // 1121
        VehicleComponent_SideSkirt, // 1122
        VehicleComponent_FrontBullbar, // 1123
        VehicleComponent_SideSkirt, // 1124
        VehicleComponent_FrontBullbar, // 1125
        VehicleComponent_Exhaust, // 1126
        VehicleComponent_Exhaust, // 1127
        VehicleComponent_Roof, // 1128
        VehicleComponent_Exhaust, // 1129
        VehicleComponent_Roof, // 1130
        VehicleComponent_Roof, // 1131
        VehicleComponent_Exhaust, // 1132
        VehicleComponent_SideSkirt, // 1133
        VehicleComponent_SideSkirt, // 1134
        VehicleComponent_Exhaust, // 1135
        VehicleComponent_Exhaust, // 1136
        VehicleComponent_SideSkirt, // 1137
        VehicleComponent_Spoiler, // 1138
        VehicleComponent_Spoiler, // 1139
        VehicleComponent_RearBumper, // 1140
        VehicleComponent_RearBumper, // 1141
        VehicleComponent_VentLeft, // 1142
        VehicleComponent_VentRight, // 1143
        VehicleComponent_VentLeft, // 1144
        VehicleComponent_VentRight, // 1145
        VehicleComponent_Spoiler, // 1146
        VehicleComponent_Spoiler, // 1147
        VehicleComponent_RearBumper, // 1148
        VehicleComponent_RearBumper, // 1149
        VehicleComponent_RearBumper, // 1150
        VehicleComponent_RearBumper, // 1151
        VehicleComponent_FrontBumper, // 1152
        VehicleComponent_FrontBumper, // 1153
        VehicleComponent_RearBumper, // 1154
        VehicleComponent_FrontBumper, // 1155
        VehicleComponent_RearBumper, // 1156
        VehicleComponent_FrontBumper, // 1157
        VehicleComponent_Spoiler, // 1158
        VehicleComponent_RearBumper, // 1159
        VehicleComponent_FrontBumper, // 1160
        VehicleComponent_RearBumper, // 1161
        VehicleComponent_Spoiler, // 1162
        VehicleComponent_Spoiler, // 1163
        VehicleComponent_Spoiler, // 1164
        VehicleComponent_FrontBumper, // 1165
        VehicleComponent_FrontBumper, // 1166
        VehicleComponent_RearBumper, // 1167
        VehicleComponent_RearBumper, // 1168
        VehicleComponent_FrontBumper, // 1169
        VehicleComponent_FrontBumper, // 1170
        VehicleComponent_FrontBumper, // 1171
        VehicleComponent_FrontBumper, // 1172
        VehicleComponent_FrontBumper, // 1173
        VehicleComponent_FrontBumper, // 1174
        VehicleComponent_FrontBumper, // 1175
        VehicleComponent_RearBumper, // 1176
        VehicleComponent_RearBumper, // 1177
        VehicleComponent_RearBumper, // 1178
        VehicleComponent_FrontBumper, // 1179
        VehicleComponent_RearBumper, // 1180
        VehicleComponent_FrontBumper, // 1181
        VehicleComponent_FrontBumper, // 1182
        VehicleComponent_RearBumper, // 1183
        VehicleComponent_RearBumper, // 1184
        VehicleComponent_FrontBumper, // 1185
        VehicleComponent_RearBumper, // 1186
        VehicleComponent_RearBumper, // 1187
        VehicleComponent_FrontBumper, // 1188
        VehicleComponent_FrontBumper, // 1189
        VehicleComponent_FrontBumper, // 1190
        VehicleComponent_FrontBumper, // 1191
        VehicleComponent_RearBumper, // 1192
        VehicleComponent_RearBumper // 1193
    };

    component -= 1000;
    if (0 <= component && component < MAX_VEHICLE_COMPONENTS) {
        return allComponentSlots[component];
    }
    return VehicleComponent_None;
};

}
