//////////////////////////////////////////////////////////////////////
//
// Copyright (c) Audiokinetic Inc. 2006-2015. All rights reserved.
//
// Audiokinetic Wwise SDK version, build number and copyright constants.
// These are used by sample projects to display the version and to
// include it in their assembly info. They can also be used by games
// or tools to display the current version and build number of the
// Wwise Sound Engine.
//
//////////////////////////////////////////////////////////////////////

using System;
using System.Text;

namespace AK
{
	namespace Wwise
	{
		public static class Version
		{
			#region Wwise SDK Version - Numeric values

			/// <summary>
			/// Wwise SDK major version
			/// </summary>
            public const int Major = 2014;

			/// <summary>
			/// Wwise SDK minor version
			/// </summary>
            public const int Minor = 1;

			/// <summary>
			/// Wwise SDK sub-minor version
			/// </summary>
            public const int SubMinor = 6;

			/// <summary>
			/// Wwise SDK build number
			/// </summary>
            public const int Build = 5318;
			
			/// <summary>
			/// Wwise SDK build nickname
			/// </summary>
			public const string Nickname = "";

			#endregion Wwise SDK Version - Numeric values

			#region Wwise SDK Version - String values

			/// <summary>
			/// String representing the Wwise SDK version
			/// </summary>
            public static string VersionName
            {
                get
                {
                    if (Nickname.Length == 0)
                        return "v2014.1.6";
                    else
                        return "v2014.1.6_" + Nickname;
                }
            }

            /// <summary>
            /// String representing the Wwise SDK version
            /// </summary>
            public const string AssemblyVersion = "2014.1.6.5318";

			/// <summary>
			/// String representing the Wwise SDK copyright notice
			/// </summary>
            public const string CopyrightNotice = "\xA9 Audiokinetic Inc. 2006-2015. All rights reserved.";

            #endregion Wwise SDK Version - String values
        }
	}
}