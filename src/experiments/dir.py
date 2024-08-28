import os


def create_directories_if_not_exist(path_list):
    for path in path_list:
        try:
            # Create a new directory if it does not exist
            if not os.path.exists(path):
                os.makedirs(path)
                print(f"Created directory: {path}")
            else:
                print(f"Directory already exists: {path}")
        except Exception as e:
            print(
                f"An error occurred while creating directory {path}: {str(e)}")


if __name__ == "__main__":
    # Specify the list of paths and call the function
    paths_to_create = [
        "../../data/test/io",
        "../../data/test/ss",
        "../../data/test/dpf",
        "../../data/test/dcf",
        "../../data/test/ddcf",
        "../../data/test/keyio",
        "../../data/test/comp",
        "../../data/test/zt",
        "../../data/test/rank",
        "../../data/test/fmi",
        "../../data/bench/rank",
        "../../data/bench/fmi",
        "../../log/test",
        "../../log/bench/dpf",
        "../../log/bench/fmi",
    ]
    current_file_path = os.path.dirname(os.path.abspath(__file__))
    paths_to_create = [current_file_path +
                       '/' + path for path in paths_to_create]
    create_directories_if_not_exist(paths_to_create)
