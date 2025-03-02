# Copyright 2021 Garena Online Private Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Rule for simple expansion of template files.

This performs a simple search over the template file for the keys in
substitutions, and replaces them with the corresponding values.

Typical usage:
::

    load("/tools/build_rules/template_rule", "expand_header_template")
    template_rule(
        name = "ExpandMyTemplate",
        src = "my.template",
        out = "my.txt",
        substitutions = {
          "$VAR1": "foo",
          "$VAR2": "bar",
        }
    )

Args:
    name: The name of the rule.
    template: The template file to expand.
    out: The destination of the expanded file.
    substitutions: A dictionary mapping strings to their substitutions.
"""

def template_rule_impl(ctx):
    """Helper function for template_rule."""
    ctx.actions.expand_template(
        template = ctx.file.src,
        output = ctx.outputs.out,
        substitutions = ctx.attr.substitutions,
    )

template_rule = rule(
    attrs = {
        "src": attr.label(
            mandatory = True,
            allow_single_file = True,
        ),
        "substitutions": attr.string_dict(mandatory = True),
        "out": attr.output(mandatory = True),
    },
    # output_to_genfiles is required for header files.
    output_to_genfiles = True,
    implementation = template_rule_impl,
)

def _copy_directory_impl(ctx):
    # Use the label as the output directory name, so that the output directory is unique (within a BUILD file)
    output_dir = ctx.actions.declare_directory(ctx.label.name)

    # Create commands that copy all input files to the output directory
    commands = []
    dest_dir = output_dir.path + "/" + ctx.attr.out
    commands.append("mkdir -p %s" % dest_dir)
    for src in ctx.files.src:
        commands.append("cp -r %s %s/" % (src.path, dest_dir))

    ctx.actions.run_shell(
        inputs = ctx.files.src,
        outputs = [output_dir],
        command = " && ".join(commands),
    )

    return [DefaultInfo(files = depset([output_dir]))]

copy_directory = rule(
    implementation = _copy_directory_impl,
    attrs = {
        "src": attr.label(
            mandatory = True,
            allow_files = True,
            doc = "Source directory or files to copy",
        ),
        "out": attr.string(
            default = "",
            doc = "Optional subdirectory path within the output directory to copy files to",
        ),
    },
)
