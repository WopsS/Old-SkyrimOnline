﻿#region

using System;
using System.IO;
using System.Reflection;
using System.Threading;
using Game.Server.Internals;
using Game.Tools.IniManager;
using log4net;
using log4net.Config;

#endregion

/*
 * Main Game.Server class
 * Create server config and server object
 * Create MaseterServerClient
 */

namespace Game.Server
{
    internal class Program
    {
        private static readonly ILog Logger = LogManager.GetLogger(MethodBase.GetCurrentMethod().DeclaringType);
        private static readonly IniLoader iniLoader = new IniLoader(ServerConfig.SERVER_CONFIG_FILE);
        private static ServerConfig config;

        public static Int32 Game = 0;

        /// <summary>
        /// </summary>
        /// <returns></returns>
        private static GameServer CreateServer()
        {
            if (!File.Exists(ServerConfig.SERVER_CONFIG_FILE))
            {
                IniWriter writer = new IniWriter(ServerConfig.SERVER_CONFIG_FILE);

                writer.addAndSelectSection(ServerConfig.SECTION_GENERAL);

                writer.addKeyToSelectedSection(ServerConfig.GENERAL_KEY_GAME, "Skyrim", "Game to set: Skyrim, Oblivion");
                writer.addKeyToSelectedSection(ServerConfig.GENERAL_KEY_NAME, "Server Name", "Your server name");
                writer.addKeyToSelectedSection(ServerConfig.GENERAL_KEY_ADDRESS, "127.0.0.1", "Local ip address or name");
                writer.addKeyToSelectedSection(ServerConfig.GENERAL_KEY_PORT, "14242", "Server port default: 14242");
                writer.addKeyToSelectedSection(ServerConfig.GENERAL_KEY_ONLINE, "true",
                    "Server online flag to enable inbound internet connection default: true");
                writer.addKeyToSelectedSection(ServerConfig.GENERAL_KEY_LANGUAGE, "English",
                    "Server language communications, defaulat: English");
                writer.addKeyToSelectedSection(ServerConfig.GENERAL_KEY_PASSWORD, "",
                    "If password is set game will be private only for players with password, defaulat: \"\" ");

                writer.addAndSelectSection(ServerConfig.SECTION_MASTER);

                writer.addKeyToSelectedSection(ServerConfig.MASTER_KEY_GUID, "",
                    "Server GUID, to generate use http://www.guidgenerator.com/, default: \"\" ");

                if (!writer.SaveFile())
                {
                    Console.WriteLine("ERROR: Server config ini file not created");
                }
            }

            config =
                new ServerConfig(
                    iniLoader.GetValue(ServerConfig.SECTION_GENERAL, ServerConfig.GENERAL_KEY_GAME, "Skyrim"),
                    iniLoader.GetValue(ServerConfig.SECTION_GENERAL, ServerConfig.GENERAL_KEY_NAME, "Server Name"),
                    iniLoader.GetValue(ServerConfig.SECTION_GENERAL, ServerConfig.GENERAL_KEY_ADDRESS, "127.0.0.1"),
                    iniLoader.GetValue(ServerConfig.SECTION_GENERAL, ServerConfig.GENERAL_KEY_PORT, 14242),
                    iniLoader.GetValue(ServerConfig.SECTION_GENERAL, ServerConfig.GENERAL_KEY_ONLINE, true),
                    iniLoader.GetValue(ServerConfig.SECTION_GENERAL, ServerConfig.GENERAL_KEY_LANGUAGE, "English"),
                    iniLoader.GetValue(ServerConfig.SECTION_GENERAL, ServerConfig.GENERAL_KEY_PASSWORD, ""),
                    iniLoader.GetValue(ServerConfig.SECTION_MASTER, ServerConfig.MASTER_KEY_GUID, "")
                    );

            return new GameServer(config);
        }

        /// <summary>
        /// </summary>
        /// <param name="server"></param>
        /// <returns></returns>
        private static MasterServerClient CreateMasterServerClient(GameServer server)
        {
            Game = config.getGame();
            return new MasterServerClient(server, config.getGUID());
        }

        /// <summary>
        /// </summary>
        /// <param name="args"></param>
        private static void Main(string[] args)
        {
            try
            {
                XmlConfigurator.Configure();

                GameServer server = CreateServer();
                MasterServerClient masterClient = CreateMasterServerClient(server);

                while (Console.KeyAvailable == false || Console.ReadKey().Key != ConsoleKey.Escape)
                {
                    server.Update();
                    masterClient.Update();

                    Thread.Sleep(30);
                }
            }
            catch (Exception ex)
            {
                Logger.Error(ex.ToString());
            }
        }
    }
}