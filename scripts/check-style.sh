#!/bin/bash
# Liara Engine - Style Checker Utility
# Usage: ./scripts/check-style.sh [options]

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

# Couleurs pour l'output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Options par défaut
FIX_FORMAT=false
FIX_TIDY=false
CHECK_ALL=false
QUIET=false
EXCLUDE_DIRS="external build .git .vscode .idea"

print_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

OPTIONS:
    -f, --fix-format    Apply clang-format fixes automatically
    -t, --fix-tidy      Apply clang-tidy fixes automatically
    -a, --all           Check all files (not just modified)
    -q, --quiet         Suppress verbose output
    -h, --help          Show this help message

EOF
}

log_info() {
    if [[ "$QUIET" != "true" ]]; then
        echo -e "${BLUE}[INFO]${NC} $1"
    fi
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    log_info "Checking dependencies..."

    local missing_deps=()

    if ! command -v clang-format &> /dev/null; then
        missing_deps+=("clang-format")
    fi

    if ! command -v clang-tidy &> /dev/null; then
        missing_deps+=("clang-tidy")
    fi

    if ! command -v git &> /dev/null; then
        missing_deps+=("git")
    fi

    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        echo "Please install them first:"
        echo "  Ubuntu/Debian: sudo apt-get install clang-format clang-tidy"
        echo "  macOS: brew install clang-format"
        echo "  Windows: Install LLVM tools"
        exit 1
    fi

    log_success "All dependencies found"
}

get_files_to_check() {
    local files=()

    if [[ "$CHECK_ALL" == "true" ]]; then
        log_info "Checking all source files..."

        # Construire la commande find avec exclusions
        local find_cmd="find \"$PROJECT_ROOT\" -type f \\( -name \"*.cpp\" -o -name \"*.h\" -o -name \"*.hpp\" -o -name \"*.tpp\" \\)"

        # Ajouter les exclusions
        for exclude_dir in $EXCLUDE_DIRS; do
            find_cmd+=" -not -path \"*/${exclude_dir}/*\""
        done

        # Exécuter et collecter les résultats
        while IFS= read -r -d '' file; do
            files+=("$file")
        done < <(eval "$find_cmd -print0")

    else
        log_info "Checking modified files..."

        # Obtenir les fichiers modifiés depuis le dernier commit
        while IFS= read -r file; do
            if [[ -f "$file" && "$file" =~ \.(cpp|h|hpp)$ ]]; then
                # Vérifier si le fichier n'est pas dans les dossiers exclus
                local should_exclude=false
                for exclude_dir in $EXCLUDE_DIRS; do
                    if [[ "$file" == *"$exclude_dir"* ]]; then
                        should_exclude=true
                        break
                    fi
                done

                if [[ "$should_exclude" == "false" ]]; then
                    files+=("$PROJECT_ROOT/$file")
                fi
            fi
        done < <(git -C "$PROJECT_ROOT" diff --name-only HEAD~1 2>/dev/null || git -C "$PROJECT_ROOT" ls-files --others --cached)
    fi

    # Filtrer les fichiers qui existent vraiment
    local existing_files=()
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            existing_files+=("$file")
        fi
    done

    printf '%s\n' "${existing_files[@]}"
}

check_clang_format() {
    local files=("$@")
    local issues=0

    if [[ ${#files[@]} -eq 0 ]]; then
        log_info "No files to check with clang-format"
        return 0
    fi

    log_info "Running clang-format on ${#files[@]} files..."

    for file in "${files[@]}"; do
        if [[ "$FIX_FORMAT" == "true" ]]; then
            # Appliquer les corrections directement
            if clang-format -i "$file"; then
                log_info "Fixed formatting: $(basename "$file")"
            else
                log_error "Failed to format: $file"
                ((issues++))
            fi
        else
            # Juste vérifier sans modifier
            if ! clang-format --dry-run --Werror "$file" &>/dev/null; then
                log_warning "Formatting issues in: $file"

                # Afficher un aperçu des différences si pas en mode quiet
                if [[ "$QUIET" != "true" ]]; then
                    echo "  Preview of changes:"
                    clang-format "$file" | diff -u "$file" - | head -20 || true
                    echo "  (Use -f to auto-fix)"
                fi
                ((issues++))
            fi
        fi
    done

    if [[ $issues -eq 0 ]]; then
        log_success "clang-format: All files are properly formatted"
    else
        log_warning "clang-format: Found $issues files with formatting issues"
    fi

    return $issues
}

check_clang_tidy() {
    local files=("$@")
    local issues=0

    if [[ ${#files[@]} -eq 0 ]]; then
        log_info "No files to check with clang-tidy"
        return 0
    fi

    # Vérifier si le fichier compile_commands.json existe
    if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
        log_info "Generating compile_commands.json..."
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug >/dev/null 2>&1 || {
            log_error "Failed to generate compile_commands.json"
            return 1
        }
        cd "$PROJECT_ROOT"
    fi

    log_info "Running clang-tidy on ${#files[@]} files..."

    for file in "${files[@]}"; do
        local tidy_output
        local tidy_exit_code=0

        if [[ "$FIX_TIDY" == "true" ]]; then
            # Appliquer les corrections automatiques
            tidy_output=$(clang-tidy "$file" -p "$BUILD_DIR" --fix --quiet 2>&1) || tidy_exit_code=$?
        else
            # Juste analyser
            tidy_output=$(clang-tidy "$file" -p "$BUILD_DIR" --quiet 2>&1) || tidy_exit_code=$?
        fi

        if [[ $tidy_exit_code -ne 0 ]] || [[ -n "$tidy_output" ]]; then
            log_warning "clang-tidy issues in: $file"

            if [[ "$QUIET" != "true" && -n "$tidy_output" ]]; then
                echo "$tidy_output" | head -10
                if [[ $(echo "$tidy_output" | wc -l) -gt 10 ]]; then
                    echo "  ... (output truncated, use clang-tidy directly for full output)"
                fi
            fi
            ((issues++))
        fi
    done

    if [[ $issues -eq 0 ]]; then
        log_success "clang-tidy: No issues found"
    else
        log_warning "clang-tidy: Found issues in $issues files"
    fi

    return $issues
}

main() {
    # Parse des arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -f|--fix-format)
                FIX_FORMAT=true
                shift
                ;;
            -t|--fix-tidy)
                FIX_TIDY=true
                shift
                ;;
            -a|--all)
                CHECK_ALL=true
                shift
                ;;
            -q|--quiet)
                QUIET=true
                shift
                ;;
            -h|--help)
                print_usage
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
        esac
    done

    # En-tête
    echo "=============================================="
    echo "    Liara Engine - Code Style Checker"
    echo "=============================================="
    echo

    # Vérifications préliminaires
    check_dependencies

    # Obtenir la liste des fichiers à vérifier
    readarray -t files_to_check < <(get_files_to_check)

    if [[ ${#files_to_check[@]} -eq 0 ]]; then
        log_info "No files to check"
        exit 0
    fi

    log_info "Found ${#files_to_check[@]} files to check"

    # Exécuter les vérifications
    local format_issues=0
    local tidy_issues=0

    echo
    echo "--- Running clang-format ---"
    check_clang_format "${files_to_check[@]}" || format_issues=$?

    echo
    echo "--- Running clang-tidy ---"
    check_clang_tidy "${files_to_check[@]}" || tidy_issues=$?

    # Résumé final
    echo
    echo "=============================================="
    echo "                 SUMMARY"
    echo "=============================================="

    local total_issues=$((format_issues + tidy_issues))

    if [[ $total_issues -eq 0 ]]; then
        log_success "All checks passed!"
        echo
        echo "Your code follows the Liara Engine style guidelines."
    else
        echo "Issues found:"
        [[ $format_issues -gt 0 ]] && echo "  - clang-format: $format_issues files"
        [[ $tidy_issues -gt 0 ]] && echo "  - clang-tidy: $tidy_issues files"
        echo
        echo "Suggestions:"
        [[ $format_issues -gt 0 ]] && echo "  - Run with -f to auto-fix formatting"
        [[ $tidy_issues -gt 0 ]] && echo "  - Run with -t to auto-fix some tidy issues"
        echo "  - See CODING_STYLE.md for guidelines"
    fi

    exit $total_issues
}

main "$@"