import os

import ansible_runner
from west.commands import WestCommand


class RemoteFlash(WestCommand):

    def __init__(self):
        super().__init__("remote_flash",
                         "Flash a Zephyr application from a remote device", "")

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(self.name,
                                         help=self.help,
                                         description=self.description)

        return parser

    def do_run(self, args, unknown_args):
        playbook_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                    "playbooks")
        ansible_runner.run(private_data_dir=playbook_dir,
                           inventory=os.path.join(playbook_dir,
                                                  "inventory.yaml"),
                           playbook="remote_flash.yaml",
                           extravars={"app_dir": os.getcwd()})
