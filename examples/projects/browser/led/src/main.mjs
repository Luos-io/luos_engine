import program from "../.pio/build/browser/program.mjs";

program()
  .then(({ Luos_Init, Led_Init, Luos_Loop, Led_Loop, Ws_Init, Ws_Loop }) => {
    Luos_Init();
    Ws_Init();
    Led_Init();

    const mainLoop = () => {
      Luos_Loop();
      Ws_Loop();
      Led_Loop();
    };

    return new Promise(() => setInterval(mainLoop, 10));
  })
  .catch((err) => {
    console.error("Error", err);
    if (typeof window === "undefined") {
      process.exit(-1);
    }
  });
