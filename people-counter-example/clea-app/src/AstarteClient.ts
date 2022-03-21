import axios from "axios";

type AstarteClientProps = {
    astarteUrl: URL;
    realm: string;
    token: string;
};

type Config = AstarteClientProps & {
    appEngineUrl: URL;
};

type CameraDataParameters = {
    deviceId: string;
    sinceAfter?: string;
    since?: Date;
    to?: Date;
    limit?: number;
};

type MultipleCameraDataParameters = {
    deviceId: string;
    since: Date;
    to: Date;
};

type CameraData = {
    people: Array<string>,
    people_count: Number,
    timestamp: string
};

class AstarteClient {
    config: Config;


    constructor({ astarteUrl, realm, token }: AstarteClientProps) {
        this.config = {
            astarteUrl,
            realm,
            token,
            appEngineUrl: new URL("/", astarteUrl),
        };
    }



    // FIXME Remove me!!!
    sleep (ms : number) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    



    async getSceneSettings ({deviceId}: {deviceId: string}) {
        const {appEngineUrl, realm, token}  = this.config;
        const interfaceName                 = "ai.clea.examples.SceneSettings";
        const path                          = `appengine/v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}`;
        const requestUrl                    = new URL(path, appEngineUrl);
        
        return axios ({
            method  : "get",
            url     : requestUrl.toString(),
            headers : {
                "Authorization" : `Bearer ${token}`,
                "Content-Type"  : "application/json;charset=UTF-8",
            }
        }).then ((response) => {
            let scene_zones     = response.data.data["scene_zones"]
            let parsed_zones    = []
            for (let i in scene_zones) {
                parsed_zones.push (JSON.parse(scene_zones[i]))
            }

            return parsed_zones
        })
    }



    async getUpdateInterval ({deviceId}: {deviceId: string}) {
        const {appEngineUrl, realm, token}  = this.config;
        const interfaceName                 = "ai.clea.examples.SceneSettings";
        const path                          = `appengine/v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}`;
        const requestUrl                    = new URL(path, appEngineUrl);
        
        return axios ({
            method  : "get",
            url     : requestUrl.toString(),
            headers : {
                "Authorization" : `Bearer ${token}`,
                "Content-Type"  : "application/json;charset=UTF-8",
            }
        }).then ((response) => {
            return response.data.data["update_interval"]
        })
    }



    async getMultipleCameraData ({deviceId, since, to}:MultipleCameraDataParameters) {
        // Retrieving camera data 12 hours for 12 jours
        const PROMISES_MAX_LENGTH               = 50;
        const { appEngineUrl, realm, token }    = this.config;
        const interfaceName                     = "ai.clea.examples.PeopleCounter";
        const path                              = `appengine/v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/camera`;
        const requestUrl                        = new URL(path, appEngineUrl.toString());

        let promises                            = [];
        let results                             = [];
        let tmp_start_date                      = new Date(since);
        let tmp_end_date                        = new Date(since);
        tmp_end_date.setHours(tmp_end_date.getHours()+12);

        while (tmp_start_date < to) {
            if (tmp_end_date>to) {
                tmp_end_date    = new Date(to)
            }
            
            const query: Record<string, string> = {};
            query.since                         = tmp_start_date.toISOString();
            query.to                            = tmp_end_date.toISOString();
            requestUrl.search = new URLSearchParams(query).toString();

            promises.push (axios({
                method  : 'get',
                url     : requestUrl.toString(),
                headers : {
                    Authorization: `Bearer ${token}`,
                    "Content-Type": "application/json;charset=UTF-8",
                }
            }))

            // Updating start and end date
            tmp_start_date.setHours(tmp_start_date.getHours()+12)
            tmp_end_date.setHours(tmp_end_date.getHours()+12);

            // Checking if promises has to be awaitied
            if (promises.length > PROMISES_MAX_LENGTH) {
                for (let i in promises) {
                    try {
                        let res = await promises[i]
                        for (let ri in res.data.data) {
                            let item        = res.data.data[ri]
                            results.push ({
                                people          : item.people,
                                people_count    : item.people_count,
                                timestamp       : item.timestamp
                            })
                        }
                    } catch (err) {
                        // Do nothing
                        console.warn (`Catched an error`)
                    }
                }
                promises    = []
            }
        }

        // Awaiting remaining promises
        for (let i in promises) {
            try {
                let res = await promises[i]
                for (let ri in res.data.data) {
                    let item        = res.data.data[ri]
                    results.push ({
                        people          : item.people,
                        people_count    : item.people_count,
                        timestamp       : item.timestamp
                    })
                }
            } catch (err) {
                // Do nothing
                console.warn (`Catched an error`)
            }
        }

        return results;
    }



    async getCameraData({deviceId, sinceAfter, since, to, limit}: CameraDataParameters) {
        const { appEngineUrl, realm, token } = this.config;
        const interfaceName = "ai.clea.examples.PeopleCounter";
        const path = `appengine/v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/camera`;
        const requestUrl = new URL(path, appEngineUrl.toString());
        const query: Record<string, string> = {};
        if (sinceAfter) {
            query.sinceAfter = sinceAfter;
        }
        if (since) {
            query.since = since.toISOString();
        }
        if (to) {
            query.to = to.toISOString();
        }
        if (limit) {
            query.limit = limit.toString();
        }
        requestUrl.search = new URLSearchParams(query).toString();
        return axios({
            method: "get",
            url: requestUrl.toString(),
            headers: {
                Authorization: `Bearer ${token}`,
                "Content-Type": "application/json;charset=UTF-8",
            },
        }).then((response) => {
            return response.data.data;
        });
    }
}

export default AstarteClient;
