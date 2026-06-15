 * |   | period         | double  | s   |   0.02        | No       | period of the main thread                       |  |
 * |   | name           | string  | -   |   /robot/part | Yes      | Prefix name of the ports opened by the device   | opened ports will be: <name><namesuffix>/rpc:i <name><namesuffix>/command:i <name><namesuffix>/state:o <name><namesuffix>/stateExt:o |
 * |   | namesuffix     | string  | -   |   /dinrail    | No       | Suffix appended to the base port prefix         | default keeps DinRail ports separated from stock YARP ports |
