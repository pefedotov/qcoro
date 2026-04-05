import json
from argparse import ArgumentParser
from typing import Any, Optional

qt6_config = {
    "archives": ["qtbase", "icu", "qtdeclarative"],
    "modules": ["qtwebsockets", "qtwebengine", "qtwebchannel", "qtpositioning"]
}

qt = [
    {
        "version": "6.8.0",
        **qt6_config
    },
    {
        "version": "6.11.0",
        **qt6_config
    }
]

platforms = [
    {
        "name": "windows",
        "compilers": [
            {"name": "msvc"},
            {"name": "clang-cl"}
        ]
    },
    {
        "name": "macos",
        "compilers": [{"name": "apple-clang"}]
    },
    {
        "name": "linux",
        "compilers": [
            {
                "name": "gcc",
                "versions": ["13", "14", "15"]
            },
            {
                "name": "clang",
                "versions": ["18", "20", "22"]
            }
        ]
    }
]


output = {
    "include": []
}


def get_os_for_platform(platform: str) -> str:
    if platform == "windows":
        return "windows-2022"
    if platform == "linux":
        return "ubuntu-latest"
    if platform == "macos":
        return "macos-15"
    raise RuntimeError(f"Invalid platform '{platform}'.")


def get_base_image_for_compiler(compiler: str) -> Optional[str]:
    if compiler == "gcc":
        return "gcc"
    if compiler == "clang":
        return "debian"
    return None


def create_configuration(
    qt: dict[str, Any], platform: str, compiler: str,
    compiler_version: str = ""
) -> dict[str, Any]:
    return {
        "qt_version": qt["version"],
        "qt_modules": ' '.join(qt["modules"]),
        "qt_archives": ' '.join(qt["archives"]),
        "platform": platform,
        "compiler": compiler,
        "compiler_base_image": get_base_image_for_compiler(compiler),
        "compiler_version": compiler_version,
        "compiler_full": compiler if not compiler_version else f"{compiler}-{compiler_version}",
        "runs_on": get_os_for_platform(platform),
        "with_qtdbus": "OFF" if platform == "macos" else "ON"
    }


parser = ArgumentParser()
parser.add_argument('--platform')
args = parser.parse_args()

filtered_platforms = list(filter(lambda p: p['name'] == args.platform, platforms))

for qt_version in qt:
    for platform in filtered_platforms:
        for compiler in platform["compilers"]:
            if "versions" in compiler:
                for compiler_version in compiler["versions"]:
                    output["include"].append(
                        create_configuration(
                            qt_version, platform["name"], compiler["name"], compiler_version
                        )
                    )
            else:
                output["include"].append(
                    create_configuration(qt_version, platform["name"], compiler["name"]))

print(json.dumps(output))
